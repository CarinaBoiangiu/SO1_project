#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *PID_FILE = ".monitor_pid";

void cleanup_and_exit() {
    if (unlink(PID_FILE) == -1) {
        perror("System Warning: Could not remove .monitor_pid during shutdown");
    } else {
        printf("\nMonitor shutting down gracefully. Removed %s.\n", PID_FILE);
    }
    exit(EXIT_SUCCESS);
}

void handle_sigint(int sig) {
    (void)sig;
    cleanup_and_exit();
}

void handle_sigusr1(int sig) {
    (void)sig;
    printf("[EVENT] A new report has been added to the city infrastructure "
           "system!\n");
}

int main() {
    pid_t my_pid = getpid();
    printf("Starting monitor_reports...\n");
    printf("Monitor PID: %d\n", my_pid);

    struct sigaction monitor_actions;
    memset(&monitor_actions, 0x00, sizeof(struct sigaction));

    monitor_actions.sa_handler = handle_sigint;
    if (sigaction(SIGINT, &monitor_actions, NULL) < 0) {
        perror("Monitor set SIGINT");
        exit(-1);
    }

    monitor_actions.sa_handler = handle_sigusr1;
    if (sigaction(SIGUSR1, &monitor_actions, NULL) < 0) {
        perror("Monitor set SIGUSR1");
        exit(-1);
    }

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
    close(fd);

    printf("Successfully initialized %s.\n", PID_FILE);
    printf("Monitor is now running in the background. Waiting for events...\n");

    while (1) {
        pause();
    }

    cleanup_and_exit();
    return 0;
}