#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void handle_start_monitor() {
    // 1. First Fork: Create hub_mon so city_hub doesn't freeze
    pid_t hub_mon_pid = fork();

    if (hub_mon_pid < 0) {
        perror("System Error: fork failed for hub_mon");
        return;
    }

    if (hub_mon_pid > 0) {
        printf("Background monitor service initiated (PID: %d).\n",
               hub_mon_pid);
        return;
    }

    // 2. Create the Pipe
    int fd[2];
    if (pipe(fd) == -1) {
        perror("System Error: pipe failed");
        exit(EXIT_FAILURE);
    }

    pid_t monitor_pid = fork();

    if (monitor_pid < 0) {
        perror("System Error: fork failed for monitor");
        exit(EXIT_FAILURE);
    }

    if (monitor_pid == 0) {

        close(fd[0]);

        if (dup2(fd[1], STDOUT_FILENO) == -1) {
            perror("System Error: dup2 failed");
            exit(EXIT_FAILURE);
        }

        close(fd[1]);

        execlp("./monitor_reports", "monitor_reports", NULL);

        perror("System Error: execlp failed for monitor_reports");
        exit(EXIT_FAILURE);
    }

    close(fd[1]);

    FILE *pipe_stream = fdopen(fd[0], "r");
    if (!pipe_stream) {
        perror("System Error: fdopen failed");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    // This loop blocks and waits for data. It automatically breaks when the
    // pipe closes.
    while (fgets(buffer, sizeof(buffer), pipe_stream) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;

        printf("\n[HUB_MON_STREAM] -> %s\n", buffer);

        printf("city_hub> ");
        fflush(stdout);
    }

    // 4. Cleanup when the monitor ends
    fclose(pipe_stream);

    printf("\n[HUB_MON_ALERT] The background monitor service has completely "
           "terminated.\n");
    printf("city_hub> ");
    fflush(stdout);

    waitpid(monitor_pid, NULL, 0);
    exit(EXIT_SUCCESS);
}

int main() {
    char input[256];

    printf("          City Hub Interactive CLI        \n");
    printf("Available commands:\n");
    printf("  start_monitor\n");
    printf("  calculate_scores <district1> [district2...]\n");
    printf("  exit\n");

    while (1) {
        printf("city_hub> ");
        fflush(stdout);

        // Read user input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; // Handle EOF (Ctrl+D) safely
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0)
            continue;

        if (strcmp(input, "exit") == 0) {
            printf("Exiting City Hub...\n");
            break;
        } else if (strcmp(input, "start_monitor") == 0) {
            handle_start_monitor();
        } else if (strncmp(input, "calculate_scores", 16) == 0) {

            printf("Calculating scores... (coming soon!)\n");
        } else {
            printf("Error: Unknown command.\n");
        }
    }

    return 0;
}