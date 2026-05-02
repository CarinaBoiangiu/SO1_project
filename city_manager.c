#include "city_manager.h"
#include "filter.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

void setup_district(char *district);
int check_permission(const char *filepath, const char *role, int require_write);
void mode_to_string(mode_t mode, char *str);

void log_operation(const char *district, const char *role, const char *user,
                   const char *command);
void handle_add(const char *district, const char *role, const char *user);
void handle_list(const char *district, const char *role, const char *user);
void handle_remove_report(const char *district, const char *role,
                          const char *user, const char *report_id_str);
void handle_view(const char *district, const char *role, const char *user,
                 const char *report_id_str);
void handle_update_threshold(const char *district, const char *role,
                             const char *user, const char *value_str);
void handle_remove_district(const char *district, const char *role);

int main(int argc, char *argv[]) {
    if (argc < 6) {
        fprintf(
            stderr,
            "Usage: %s --role <role> --user <user> --<command> <district_id> "
            "[args...]\n",
            argv[0]);
        return 1;
    }

    char *role = NULL;
    char *user = NULL;
    char *command = NULL;
    char *district = NULL;
    char *extra_arg = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc)
            role = argv[++i];
        else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc)
            user = argv[++i];
        else if (strncmp(argv[i], "--", 2) == 0) {
            command = argv[i] + 2;
            if (i + 1 < argc)
                district = argv[++i];
            if (i + 1 < argc && strncmp(argv[i + 1], "--", 2) != 0)
                extra_arg = argv[++i];
        }
    }

    if (!role || !user || !command || !district) {
        fprintf(stderr, "Missing required arguments.\n");
        return 1;
    }

    setup_district(district);

    if (strcmp(command, "add") == 0) {
        handle_add(district, role, user);
    } else if (strcmp(command, "list") == 0) {
        handle_list(district, role, user);
    } else if (strcmp(command, "view") == 0) {
        handle_view(district, role, user, extra_arg);
    } else if (strcmp(command, "remove_report") == 0) {
        handle_remove_report(district, role, user, extra_arg);
    } else if (strcmp(command, "update_threshold") == 0) {
        handle_update_threshold(district, role, user, extra_arg);
    } else if (strcmp(command, "remove_district") == 0) {
        handle_remove_district(district, role);
    } else if (strcmp(command, "filter") == 0) {
        if (!extra_arg) {
            fprintf(stderr, "Error: Missing condition for filter command.\n");
            return 1;
        }
        filter_reports(district, role, user, extra_arg);
    } else {
        fprintf(stderr, "Error: Unknown command '--%s'\n", command);
        return 1;
    }

    return 0;
}

void setup_district(char *district) {
    struct stat st = {0};

    if (stat(district, &st) == -1) {
        mkdir(district, 0750);
    }
    chmod(district, 0750);

    char path[256];
    int fd;

    snprintf(path, sizeof(path), "%s/reports.dat", district);
    fd = open(path, O_CREAT | O_RDWR | O_APPEND, 0664);
    if (fd != -1) {
        fchmod(fd, 0664);
        close(fd);
    }

    snprintf(path, sizeof(path), "%s/district.cfg", district);
    if (stat(path, &st) == -1) {
        fd = open(path, O_CREAT | O_WRONLY, 0640);
        if (fd != -1) {
            fchmod(fd, 0640);
            const char *default_cfg = "2\n";
            write(fd, default_cfg, strlen(default_cfg));
            close(fd);
        }
    } else {
        chmod(path, 0640);
    }

    snprintf(path, sizeof(path), "%s/logged_district", district);
    fd = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd != -1) {
        fchmod(fd, 0644);
        close(fd);
    }

    char symlink_name[256];
    snprintf(symlink_name, sizeof(symlink_name), "active_reports-%s", district);

    unlink(symlink_name);

    snprintf(path, sizeof(path), "%s/reports.dat", district);
    symlink(path, symlink_name);
}

int check_permission(const char *filepath, const char *role,
                     int require_write) {
    struct stat file_stat;
    if (stat(filepath, &file_stat) < 0) {
        perror("stat failed");
        return 0;
    }

    mode_t mode = file_stat.st_mode;

    if (strcmp(role, "manager") == 0) {
        if (require_write && !(mode & S_IWUSR))
            return 0;
        if (!require_write && !(mode & S_IRUSR))
            return 0;
        return 1;
    } else if (strcmp(role, "inspector") == 0) {
        if (require_write && !(mode & S_IWGRP))
            return 0;
        if (!require_write && !(mode & S_IRGRP))
            return 0;
        return 1;
    }

    return 0;
}

void mode_to_string(mode_t mode, char *str) {
    strcpy(str, "---------");
    if (mode & S_IRUSR)
        str[0] = 'r';
    if (mode & S_IWUSR)
        str[1] = 'w';
    if (mode & S_IXUSR)
        str[2] = 'x';
    if (mode & S_IRGRP)
        str[3] = 'r';
    if (mode & S_IWGRP)
        str[4] = 'w';
    if (mode & S_IXGRP)
        str[5] = 'x';
    if (mode & S_IROTH)
        str[6] = 'r';
    if (mode & S_IWOTH)
        str[7] = 'w';
    if (mode & S_IXOTH)
        str[8] = 'x';
}

void log_operation(const char *district, const char *role, const char *user,
                   const char *command) {
    char log_path[FILE_PATH_SIZE];
    snprintf(log_path, sizeof(log_path), "%s/logged_district", district);

    if (!check_permission(log_path, role, 1)) {
        fprintf(stderr,
                "Access Denied: Role '%s' does not have write access to %s. "
                "Aborting operation.\n",
                role, log_path);
        exit(EXIT_FAILURE);
    }

    int fd = open(log_path, O_WRONLY | O_APPEND);
    if (fd == -1) {
        perror("System Error: Failed to open logged_district");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    time_t now = time(NULL);

    int len = snprintf(buffer, sizeof(buffer), "%ld\n%s\n%s %s\n", now, user,
                       role, command);

    if (write(fd, buffer, len) != len) {
        perror("System Error: Failed to write complete log entry");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (close(fd) == -1) {
        perror("System Error: Failed to close logged_district");
    }
}

void handle_add(const char *district, const char *role, const char *user) {
    char filepath[FILE_PATH_SIZE];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district);

    if (!check_permission(filepath, role, 1)) {
        fprintf(stderr,
                "Access Denied: User %s (Role: %s) cannot write to %s\n", user,
                role, filepath);
        exit(EXIT_FAILURE);
    }

    struct stat st;
    if (stat(filepath, &st) == -1) {
        perror("System Error: stat() failed on reports.dat");
        exit(EXIT_FAILURE);
    }
    int next_id = st.st_size / REPORT_SIZE;

    Report new_report;
    memset(&new_report, 0, sizeof(Report));
    new_report.id = next_id;
    new_report.timestamp = time(NULL);
    strncpy(new_report.inspector, user, MAX_INSPECTOR - 1);

    printf("X: ");
    if (scanf("%f", &new_report.latitude) != 1) {
        fprintf(stderr, "Invalid coordinate.\n");
        exit(EXIT_FAILURE);
    }

    printf("Y: ");
    if (scanf("%f", &new_report.longitude) != 1) {
        fprintf(stderr, "Invalid coordinate.\n");
        exit(EXIT_FAILURE);
    }

    printf("Category (road/lighting/flooding/other): ");
    if (scanf("%23s", new_report.category) != 1) {
        fprintf(stderr, "Invalid category.\n");
        exit(EXIT_FAILURE);
    }

    printf("Severity level (1/2/3): ");
    if (scanf("%d", &new_report.severity) != 1) {
        fprintf(stderr, "Invalid severity.\n");
        exit(EXIT_FAILURE);
    }

    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;

    printf("Description: ");
    if (fgets(new_report.description, MAX_DESCRIPTION, stdin) != NULL) {
        new_report.description[strcspn(new_report.description, "\n")] = 0;
    }

    log_operation(district, role, user, "add");

    int fd = open(filepath, O_WRONLY | O_APPEND);
    if (fd == -1) {
        perror("System Error: open() failed on reports.dat for appending");
        exit(EXIT_FAILURE);
    }

    if (write(fd, &new_report, sizeof(Report)) != sizeof(Report)) {
        perror("System Error: write() failed to append full record");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (close(fd) == -1) {
        perror("System Error: close() failed on reports.dat");
    }
}

void handle_list(const char *district, const char *role, const char *user) {
    char filepath[FILE_PATH_SIZE];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district);

    if (!check_permission(filepath, role, 0)) {
        fprintf(stderr, "Access Denied: User %s (Role: %s) cannot read %s\n",
                user, role, filepath);
        exit(EXIT_FAILURE);
    }

    log_operation(district, role, user, "list");

    struct stat st;
    if (stat(filepath, &st) == -1) {
        perror("System Error: stat() failed on reports.dat");
        exit(EXIT_FAILURE);
    }

    char perms_str[10];
    mode_to_string(st.st_mode, perms_str);
    char time_str[64];
    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("=== FILE INFO ===\n");
    printf("Permissions: %s | Size: %ld bytes | Last Modified: %s\n", perms_str,
           (long)st.st_size, time_str);
    printf("=================\n\n");

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("System Error: open() failed on reports.dat for reading");
        exit(EXIT_FAILURE);
    }

    Report r;
    int count = 0;

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        char rec_time[64];
        struct tm *rtm = localtime(&r.timestamp);
        strftime(rec_time, sizeof(rec_time), "%Y-%m-%d %H:%M:%S", rtm);

        printf("Report ID: %d\n", r.id);
        printf("Inspector: %s\n", r.inspector);
        printf("Category : %s\n", r.category);
        printf("Severity : %d\n", r.severity);
        printf("Location : (%.4f, %.4f)\n", r.latitude, r.longitude);
        printf("Reported : %s\n", rec_time);
        printf("Details  : %s\n", r.description);
        printf("--------------------------------------------------\n");

        count++;
    }

    if (count == 0) {
        printf("No reports currently logged in district '%s'.\n", district);
    } else {
        printf("Total reports: %d\n", count);
    }

    if (close(fd) == -1) {
        perror("System Error: close() failed on reports.dat");
    }
}

void handle_remove_report(const char *district, const char *role,
                          const char *user, const char *report_id_str) {

    if (strcmp(role, "manager") != 0) {
        fprintf(stderr, "Access Denied: Only managers can remove reports.\n");
        exit(EXIT_FAILURE);
    }

    if (!report_id_str) {
        fprintf(stderr,
                "Error: Missing report_id argument for remove_report.\n");
        exit(EXIT_FAILURE);
    }
    int target_id = atoi(report_id_str);

    char filepath[FILE_PATH_SIZE];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district);

    if (!check_permission(filepath, role, 1)) {
        fprintf(stderr,
                "Access Denied: Manager %s does not have write access to %s\n",
                user, filepath);
        exit(EXIT_FAILURE);
    }

    log_operation(district, role, user, "remove_report");

    struct stat st_before;
    if (stat(filepath, &st_before) == -1) {
        perror("System Error: stat() failed before removal");
        exit(EXIT_FAILURE);
    }
    printf("File size BEFORE removal: %ld bytes\n", (long)st_before.st_size);

    int fd = open(filepath, O_RDWR);
    if (fd == -1) {
        perror("System Error: open() failed for remove_report");
        exit(EXIT_FAILURE);
    }

    off_t total_records = st_before.st_size / REPORT_SIZE;
    off_t target_index = -1;
    Report temp_report;

    for (off_t i = 0; i < total_records; i++) {
        if (read(fd, &temp_report, REPORT_SIZE) != REPORT_SIZE) {
            perror("System Error: read() failed during search");
            close(fd);
            exit(EXIT_FAILURE);
        }
        if (temp_report.id == target_id) {
            target_index = i;
            break;
        }
    }

    if (target_index == -1) {
        printf("Report ID %d not found in district '%s'.\n", target_id,
               district);
        close(fd);
        return;
    }

    for (off_t i = target_index + 1; i < total_records; i++) {

        lseek(fd, i * REPORT_SIZE, SEEK_SET);
        if (read(fd, &temp_report, REPORT_SIZE) != REPORT_SIZE) {
            perror("System Error: read() failed during shift");
            close(fd);
            exit(EXIT_FAILURE);
        }

        lseek(fd, (i - 1) * REPORT_SIZE, SEEK_SET);
        if (write(fd, &temp_report, REPORT_SIZE) != REPORT_SIZE) {
            perror("System Error: write() failed during shift");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    off_t new_size = (total_records - 1) * REPORT_SIZE;
    if (ftruncate(fd, new_size) == -1) {
        perror("System Error: ftruncate() failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);

    struct stat st_after;
    if (stat(filepath, &st_after) == -1) {
        perror("System Error: stat() failed after removal");
        exit(EXIT_FAILURE);
    }
    printf("File size AFTER removal: %ld bytes\n", (long)st_after.st_size);
    printf("Successfully removed report ID %d.\n", target_id);
}

void handle_view(const char *district, const char *role, const char *user,
                 const char *report_id_str) {
    if (!report_id_str) {
        fprintf(stderr,
                "Error: Missing report_id argument for view command.\n");
        exit(EXIT_FAILURE);
    }
    int target_id = atoi(report_id_str);

    char filepath[FILE_PATH_SIZE];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district);

    if (!check_permission(filepath, role, 0)) {
        fprintf(stderr, "Access Denied: User %s (Role: %s) cannot read %s\n",
                user, role, filepath);
        exit(EXIT_FAILURE);
    }

    log_operation(district, role, user, "view");

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("System Error: open() failed on reports.dat for viewing");
        exit(EXIT_FAILURE);
    }

    Report r;
    int found = 0;

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.id == target_id) {
            char rec_time[64];
            struct tm *rtm = localtime(&r.timestamp);
            strftime(rec_time, sizeof(rec_time), "%Y-%m-%d %H:%M:%S", rtm);

            printf("=== Report Details ===\n");
            printf("Report ID: %d\n", r.id);
            printf("Inspector: %s\n", r.inspector);
            printf("Category : %s\n", r.category);
            printf("Severity : %d\n", r.severity);
            printf("Location : (%.4f, %.4f)\n", r.latitude, r.longitude);
            printf("Reported : %s\n", rec_time);
            printf("Details  : %s\n", r.description);
            printf("======================\n");

            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Report ID %d not found in district '%s'.\n", target_id,
               district);
    }

    if (close(fd) == -1) {
        perror("System Error: close() failed on reports.dat");
    }
}

void handle_update_threshold(const char *district, const char *role,
                             const char *user, const char *value_str) {

    if (strcmp(role, "manager") != 0) {
        fprintf(stderr,
                "Access Denied: Only managers can update the threshold.\n");
        exit(EXIT_FAILURE);
    }

    if (!value_str) {
        fprintf(
            stderr,
            "Error: Missing threshold value argument for update_threshold.\n");
        exit(EXIT_FAILURE);
    }

    char filepath[FILE_PATH_SIZE];
    snprintf(filepath, sizeof(filepath), "%s/district.cfg", district);

    struct stat st;
    if (stat(filepath, &st) == -1) {
        perror("System Error: stat() failed on district.cfg");
        exit(EXIT_FAILURE);
    }

    if ((st.st_mode & 0777) != 0640) {
        fprintf(stderr, "Diagnostic Error: Security breach or misconfiguration "
                        "detected!\n");
        fprintf(stderr,
                "Permissions for %s have been altered. Expected 0640, found "
                "%04o.\n",
                filepath, (st.st_mode & 0777));
        fprintf(stderr, "Refusing operation.\n");
        exit(EXIT_FAILURE);
    }

    log_operation(district, role, user, "update_threshold");

    int fd = open(filepath, O_WRONLY | O_TRUNC);
    if (fd == -1) {
        perror("System Error: open() failed on district.cfg");
        exit(EXIT_FAILURE);
    }

    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "%s\n", value_str);

    if (write(fd, buffer, len) != len) {
        perror("System Error: write() failed to update district.cfg");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Successfully updated severity threshold to %s for district '%s'.\n",
           value_str, district);

    if (close(fd) == -1) {
        perror("System Error: close() failed on district.cfg");
    }
}
void handle_remove_district(const char *district, const char *role) {

    if (strcmp(role, "manager") != 0) {
        fprintf(
            stderr,
            "Access Denied: Only managers can remove an entire district.\n");
        exit(EXIT_FAILURE);
    }

    if (strchr(district, '/') != NULL || strcmp(district, ".") == 0 ||
        strcmp(district, "..") == 0 || strlen(district) == 0) {
        fprintf(stderr, "Critical Error: Invalid district name. Path traversal "
                        "or empty string detected.\n");
        exit(EXIT_FAILURE);
    }

    char symlink_name[FILE_PATH_SIZE];
    snprintf(symlink_name, sizeof(symlink_name), "active_reports-%s", district);

    if (unlink(symlink_name) == -1) {
        perror("System Warning: Could not remove symlink (may already  be "
               "deleted)");
    } else {
        printf("Symlink '%s' succesfully removed.\n", symlink_name);
    }

    printf("Initiating recursive deletion of ditrict: %s... \n", district);
    pid_t pid = fork();

    if (pid < 0) {
        perror("System Error:  fork() failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execlp("rm", "rm", "-rf", district, NULL);
        perror("System Error: execlp() failed to execute rm");
        exit(EXIT_FAILURE);
    } else {
        int status;

        if (wait(&status) == -1) {
            perror("System Error: wait() failed");
        } else {
            if (status == 0) {
                printf("District '%s' and all its contents were successfully "
                       "removed.\n",
                       district);
            } else {
                fprintf(stderr, "Error: the 'rm -rf' command did not execute "
                                "successfully.\n");
            }
        }
    }
}
