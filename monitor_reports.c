#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *PID_FILE = ".monitor_pid";

void cleanup_and_exit() {
    if (unlink(PID_FILE) == -1) {
        printf("MSG:WARNING:Could not remove .monitor_pid during shutdown.\n");
    } else {
        printf("MSG:SHUTDOWN:Monitor shutting down gracefully. Removed %s.\n",
               PID_FILE);
    }
    exit(EXIT_SUCCESS);
}

void handle_sigint(int sig) {
    (void)sig;
    cleanup_and_exit();
}

void handle_sigusr1(int sig) {
    (void)sig;
    // Structured message for the hub to easily parse
    printf("MSG:EVENT:A new report has been added to the city infrastructure "
           "system!\n");
}

int main() {

    setvbuf(stdout, NULL, _IOLBF, 0);

    int check_fd = open(PID_FILE, O_RDONLY);
    if (check_fd != -1) {
        char pid_str[32] = {0};
        if (read(check_fd, pid_str, sizeof(pid_str) - 1) > 0) {
            pid_t existing_pid = atoi(pid_str);

            if (existing_pid > 0 && kill(existing_pid, 0) == 0) {
                printf("MSG:ERROR:Another monitor is already running with PID "
                       "%d.\n",
                       existing_pid);
                close(check_fd);
                exit(1);
            }
        }
        close(check_fd);
    }

    pid_t my_pid = getpid();
    printf("MSG:INFO:Starting monitor_reports...\n");
    printf("MSG:STARTUP:Monitor PID: %d\n", my_pid);

    struct sigaction monitor_actions;
    memset(&monitor_actions, 0x00, sizeof(struct sigaction));

    monitor_actions.sa_handler = handle_sigint;
    if (sigaction(SIGINT, &monitor_actions, NULL) < 0) {
        printf("MSG:ERROR:Monitor set SIGINT failed\n");
        exit(-1);
    }

    monitor_actions.sa_handler = handle_sigusr1;
    if (sigaction(SIGUSR1, &monitor_actions, NULL) < 0) {
        printf("MSG:ERROR:Monitor set SIGUSR1 failed\n");
        exit(-1);
    }

    int fd = open(PID_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        printf("MSG:ERROR:Failed to create or open .monitor_pid\n");
        exit(EXIT_FAILURE);
    }

    char pid_str[32];
    int len = snprintf(pid_str, sizeof(pid_str), "%d\n", my_pid);
    if (write(fd, pid_str, len) != len) {
        printf("MSG:ERROR:Failed to write complete PID to file\n");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);

    printf("MSG:INFO:Successfully initialized %s.\n", PID_FILE);
    printf("MSG:INFO:Monitor is now running in the background. Waiting for "
           "events...\n");

    while (1) {
        pause();
    }

    return 0;
}