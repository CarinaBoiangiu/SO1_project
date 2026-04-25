#include "city_manager.h"
#include "filter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>


void setup_district(char* district) ;
int check_permission(const char *filepath, const char *role, int require_write);
void mode_to_string(mode_t mode, char *str);

int main(int argc, char* argv[]) {
    if (argc < 6) {
        fprintf(stderr, "Usage: %s --role <role> --user <user> --<command> <district_id> [args...]\n", argv[0]);
        return 1;
    }
    
    char* role = NULL;
    char* user = NULL;
    char* command = NULL;
    char* district = NULL;
    char* extra_arg = NULL; 

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) role = argv[++i];
        else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) user = argv[++i];
        else if (strncmp(argv[i], "--", 2) == 0) {
            command = argv[i] + 2; 
            if (i + 1 < argc) district = argv[++i];
            if (i + 1 < argc && strncmp(argv[i+1], "--", 2) != 0) extra_arg = argv[++i]; 
        }
    }

    if (!role || !user || !command || !district) {
        fprintf(stderr, "Missing required arguments.\n");
        return 1;
    }

    setup_district(district);

    if (strcmp(command, "add") == 0) {
        char filepath[FILE_PATH_SIZE];
        snprintf(filepath, sizeof(filepath), "%s/reports.dat", district);
            
        if (!check_permission(filepath, role, 1)) {
            fprintf(stderr, "Error: User %s with role %s does not have write access to %s\n", user, role, filepath);
            return 1;
        }
        printf("Adding report to %s...\n", district);
        
    } else if (strcmp(command, "remove_report") == 0) {
        // Remove logic
    } else if (strcmp(command, "filter") == 0) {
        // Filter logic
    }

    return 0;
}


void setup_district(char* district) {
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
            const char* default_cfg = "2\n";
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

int check_permission(const char *filepath, const char *role, int require_write) {
    struct stat file_stat;
    if (stat(filepath, &file_stat) < 0) {
        perror("stat failed");
        return 0;
    }

    mode_t mode = file_stat.st_mode;

    if (strcmp(role, "manager") == 0) {
        if (require_write && !(mode & S_IWUSR)) return 0;
        if (!require_write && !(mode & S_IRUSR)) return 0;
        return 1;
    } else if (strcmp(role, "inspector") == 0) {
        if (require_write && !(mode & S_IWGRP)) return 0;
        if (!require_write && !(mode & S_IRGRP)) return 0;
        return 1;
    }
    
    return 0;
}

void mode_to_string(mode_t mode, char *str) {
    strcpy(str, "---------");
    if (mode & S_IRUSR) str[0] = 'r';
    if (mode & S_IWUSR) str[1] = 'w';
    if (mode & S_IXUSR) str[2] = 'x';
    if (mode & S_IRGRP) str[3] = 'r';
    if (mode & S_IWGRP) str[4] = 'w';
    if (mode & S_IXGRP) str[5] = 'x';
    if (mode & S_IROTH) str[6] = 'r';
    if (mode & S_IWOTH) str[7] = 'w';
    if (mode & S_IXOTH) str[8] = 'x';
}