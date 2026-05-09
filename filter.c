#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "city_manager.h"
#include "filter.h"

int filter_reports(const char *district, const char *role, const char *user,
                   int condition_count, char **conditions) {
    char filepath[FILE_PATH_SIZE];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district);

    if (!check_permission(filepath, role, 0)) {
        fprintf(stderr, "Access Denied: User %s (Role: %s) cannot read %s\n",
                user, role, filepath);
        exit(EXIT_FAILURE);
    }

    log_operation(district, role, user, "filter");

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("System Error: open() failed on reports.dat for filtering");
        exit(EXIT_FAILURE);
    }

    Report r;
    int match_count = 0;

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        int all_match = 1;

        for (int i = 0; i < condition_count; i++) {
            char field[32], op[4], value[64];

            if (!parse_condition(conditions[i], field, op, value)) {
                fprintf(stderr, "Error: Invalid condition format: %s\n",
                        conditions[i]);
                close(fd);
                exit(EXIT_FAILURE);
            }

            if (!match_condition(&r, field, op, value)) {
                all_match = 0;
                break;
            }
        }

        if (all_match) {
            printf("ID: %d | Cat: %s | Sev: %d | Inspector: %s | Desc: %s\n",
                   r.id, r.category, r.severity, r.inspector, r.description);
            match_count++;
        }
    }

    if (match_count == 0) {
        printf("No reports matched the conditions.\n");
    } else {
        printf("-----------------------------------\n");
        printf("Total matches: %d\n", match_count);
    }

    close(fd);
    return 0;
}

// AI-Assisted Function 1:
int parse_condition(const char *input, char *field, char *op, char *value) {
    if (input == NULL || field == NULL || op == NULL || value == NULL) {
        return 0;
    }

    if (sscanf(input, "%[^:]:%[^:]:%s", field, op, value) == 3) {
        return 1;
    }
    return 0;
}

// AI-Assisted Function 2:
int match_condition(Report *r, const char *field, const char *op,
                    const char *value) {
    if (!r || !field || !op || !value)
        return 0;

    if (strcmp(field, "severity") == 0) {
        int target_val = atoi(value);
        if (strcmp(op, "==") == 0)
            return r->severity == target_val;
        if (strcmp(op, "!=") == 0)
            return r->severity != target_val;
        if (strcmp(op, "<") == 0)
            return r->severity < target_val;
        if (strcmp(op, "<=") == 0)
            return r->severity <= target_val;
        if (strcmp(op, ">") == 0)
            return r->severity > target_val;
        if (strcmp(op, ">=") == 0)
            return r->severity >= target_val;
    } else if (strcmp(field, "category") == 0) {
        if (strcmp(op, "==") == 0)
            return strcmp(r->category, value) == 0;
        if (strcmp(op, "!=") == 0)
            return strcmp(r->category, value) != 0;
    } else if (strcmp(field, "inspector") == 0) {
        if (strcmp(op, "==") == 0)
            return strcmp(r->inspector, value) == 0;
        if (strcmp(op, "!=") == 0)
            return strcmp(r->inspector, value) != 0;
    } else if (strcmp(field, "timestamp") == 0) {
        time_t target_val = (time_t)atol(value);
        if (strcmp(op, "==") == 0)
            return r->timestamp == target_val;
        if (strcmp(op, "!=") == 0)
            return r->timestamp != target_val;
        if (strcmp(op, "<") == 0)
            return r->timestamp < target_val;
        if (strcmp(op, "<=") == 0)
            return r->timestamp <= target_val;
        if (strcmp(op, ">") == 0)
            return r->timestamp > target_val;
        if (strcmp(op, ">=") == 0)
            return r->timestamp >= target_val;
    }

    return 0;
}