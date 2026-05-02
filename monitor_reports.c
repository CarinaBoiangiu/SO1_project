#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

const char *PID_FILE = ".monitor_pid";

void cleanup_and_exit() {
    if (unlink(PID_FILE) == -1) {
        perror("System Warning: Could not remove .monitor_pid during shutdown");
    } else {
        printf("\nMonitor shutting down gracefully. Removed %s.\n", PID_FILE);
    }
    exit(EXIT_SUCCESS);
}

int main() {
    pid_t my_pid = getpid();
    printf("Starting monitor_reports...\n");
    printf("Monitor PID: %d\n", my_pid);

    int fd = open(PID_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("System Error: Failed to create or open .monitor_pid");
        exit(EXIT_FAILURE);
    }

    char pid_str[32];
    int len = snprintf(pid_str, sizeof(pid_str), "%d\n", my_pid);

    if (write(fd, pid_str, len) != len) {
        perror("System Error: Failed to write complete PID to file");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (close(fd) == -1) {
        perror("System Error: Failed to close .monitor_pid");
    }

    printf("Successfully initialized %s.\n", PID_FILE);
    printf("Monitor is now running in the background. Waiting for events...\n");

    while (1) {
        sleep(1);
    }

    cleanup_and_exit();
    return 0;
}