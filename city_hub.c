#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 50

void handle_start_monitor();
void handle_calculate_scores(char *districts[], int count);

int main() {
    char input[512];

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
            break;
        }

        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0)
            continue;

        // Split the input string into tokens separated by spaces
        char *cmd = strtok(input, " ");
        if (!cmd)
            continue;

        if (strcmp(cmd, "exit") == 0) {
            printf("Exiting City Hub...\n");
            break;
        } else if (strcmp(cmd, "start_monitor") == 0) {
            handle_start_monitor();
        } else if (strcmp(cmd, "calculate_scores") == 0) {
            // Extract the remaining tokens as district arguments
            char *args[MAX_ARGS];
            int arg_count = 0;
            char *token;

            while ((token = strtok(NULL, " ")) != NULL &&
                   arg_count < MAX_ARGS) {
                args[arg_count++] = token;
            }

            handle_calculate_scores(args, arg_count);
        } else {
            printf("Error: Unknown command.\n");
        }
    }

    return 0;
}

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

    // --- CHILD (hub_mon) ---
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
        // GRANDCHILD (monitor_reports)
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

    // Back in CHILD (hub_mon)
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

        printf("\n");

        if (strncmp(buffer, "MSG:EVENT:", 10) == 0) {
            printf("[\033[1;34mEVENT\033[0m] %s\n", buffer + 10);

        } else if (strncmp(buffer, "MSG:INFO:", 9) == 0) {
            printf("[\033[1;32mINFO\033[0m] %s\n", buffer + 9);

        } else if (strncmp(buffer, "MSG:ERROR:", 10) == 0) {
            printf("[\033[1;31mERROR\033[0m] %s\n", buffer + 10);

        } else if (strncmp(buffer, "MSG:STARTUP:", 12) == 0) {
            printf("[\033[1;36mSYSTEM\033[0m] %s\n", buffer + 12);

        } else if (strncmp(buffer, "MSG:SHUTDOWN:", 13) == 0) {
            printf("[\033[1;33mSTOP\033[0m] %s\n", buffer + 13);

        } else if (strncmp(buffer, "MSG:WARNING:", 12) == 0) {
            printf("[\033[1;35mWARNING\033[0m] %s\n", buffer + 12);

        } else {
            // Fallback for unexpected messages
            printf("[UNKNOWN] %s\n", buffer);
        }

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

void handle_calculate_scores(char *districts[], int count) {
    if (count == 0) {
        printf("Error: Please provide at least one district name.\n");
        printf("Usage: calculate_scores <district1> [district2 ...]\n");
        return;
    }

    // Arrays to keep track of our child processes and their pipes
    int read_pipes[MAX_ARGS];
    pid_t pids[MAX_ARGS];

    // 1. Spawning Phase
    for (int i = 0; i < count; i++) {
        int fd[2];
        if (pipe(fd) == -1) {
            perror("System Error: pipe failed");
            return;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("System Error: fork failed for scorer");
            return;
        }

        if (pid == 0) {
            // --- CHILD (Scorer) ---
            close(fd[0]);

            // Redirect stdout to the pipe
            if (dup2(fd[1], STDOUT_FILENO) == -1) {
                perror("System Error: dup2 failed");
                exit(EXIT_FAILURE);
            }
            close(fd[1]);

            execlp("./scorer", "scorer", districts[i], NULL);

            perror("System Error: execlp failed for scorer");
            exit(EXIT_FAILURE);
        } else {
            // --- PARENT (city_hub) ---
            close(fd[1]);
            read_pipes[i] = fd[0];
            pids[i] = pid;
        }
    }

    // 2. Collection Phase (Concurrent multiplexing with select)
    printf("          COMBINED WORKLOAD REPORT        \n");

    int active_pipes = count;
    int pipe_active[MAX_ARGS];
    for (int i = 0; i < count; i++) {
        pipe_active[i] = 1; // Mark all pipes as active initially
    }

    char buffer[512];

    while (active_pipes > 0) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        int max_fd = -1;

        for (int i = 0; i < count; i++) {
            if (pipe_active[i]) {
                FD_SET(read_pipes[i], &read_fds);
                if (read_pipes[i] > max_fd) {
                    max_fd = read_pipes[i];
                }
            }
        }

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("System Error: select() failed");
            break;
        }

        for (int i = 0; i < count; i++) {
            if (pipe_active[i] && FD_ISSET(read_pipes[i], &read_fds)) {
                ssize_t bytes_read =
                    read(read_pipes[i], buffer, sizeof(buffer) - 1);

                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    printf("%s", buffer);
                } else {
                    close(read_pipes[i]);
                    pipe_active[i] = 0;
                    active_pipes--;
                }
            }
        }
    }

    for (int i = 0; i < count; i++) {
        waitpid(pids[i], NULL, 0);
    }
}
