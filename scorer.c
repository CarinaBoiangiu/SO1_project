#include "city_manager.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char name[MAX_INSPECTOR];
    int total_severity;
} InspectorScore;

#define MAX_UNIQUE_INSPECTORS 100

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <district_id>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *district = argv[1];
    char filepath[FILE_PATH_SIZE];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district);

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        printf("[%s] Error: Could not access reports.dat (District might not "
               "exist)\n",
               district);
        return EXIT_SUCCESS;
    }

    InspectorScore scores[MAX_UNIQUE_INSPECTORS];
    int num_inspectors = 0;
    Report r;

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        int found = 0;

        for (int i = 0; i < num_inspectors; i++) {
            if (strcmp(scores[i].name, r.inspector) == 0) {
                scores[i].total_severity += r.severity;
                found = 1;
                break;
            }
        }

        if (!found && num_inspectors < MAX_UNIQUE_INSPECTORS) {
            strncpy(scores[num_inspectors].name, r.inspector,
                    MAX_INSPECTOR - 1);
            scores[num_inspectors].name[MAX_INSPECTOR - 1] = '\0';
            scores[num_inspectors].total_severity = r.severity;
            num_inspectors++;
        }
    }

    close(fd);

    printf(" Workload Summary: District '%s' \n", district);
    if (num_inspectors == 0) {
        printf("  No reports filed in this district.\n");
    } else {
        for (int i = 0; i < num_inspectors; i++) {
            printf("  Inspector: %-15s | Total Workload Score: %d\n",
                   scores[i].name, scores[i].total_severity);
        }
    }
    printf("\n");

    return EXIT_SUCCESS;
}