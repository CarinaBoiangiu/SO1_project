# AI usage

## The Tool used

-  Google gemini


## What The AI was used for

### Phase 1

1. The AI was used for generating the Makefile for the project
    - Prompt used: 

    ```txt
    Make me a makefile for a c project
    use wildcards so there won't be needed to add more file definitions in the future
    all files will be located in the same directory
    target is named city_manager
    add clean for objs and target
    ```

    - Answer generated:

    ```txt
    # Compiler and flags
    CC = gcc
    CFLAGS = -Wall -Wextra -g

    # Automatically find all .c files in the current directory
    SRCS = $(wildcard *.c)

    # Generate a list of object files by replacing .c with .o
    OBJS = $(SRCS:.c=.o)

    # Output binary name
    TARGET = city_manager

    # Default target
    all: $(TARGET)

    # Link all object files to create the final executable
    $(TARGET): $(OBJS)
        $(CC) $(CFLAGS) -o $@ $^

    # Pattern rule to compile each .c file into a .o file
    %.o: %.c
        $(CC) $(CFLAGS) -c $< -o $@

    # Clean up build artifacts and generated district data
    clean:
        rm -f $(TARGET) $(OBJS)

    .PHONY: all clean
    ```

2. Using AI for generating the function `int parse_condition(const char *input, char *field, char *op, char *value);` for the project:
   -  Promt used:
    ```txt
    Generate a function `int parse_condition(const char *input, char *field, char *op, char *value);` which splits a field:operator:value string into its three parts.
    Function returns 1 if conditions is parse is successful and 0 the other way
    ```
   -  Answer generated:
    ```c
    int parse_condition(const char *input, char *field, char *op, char *value) {
    // We use sscanf to parse the format "field:operator:value"
    // %[^:] means "read everything until you hit a colon"
    if (sscanf(input, "%[^:]:%[^:]:%s", field, op, value) == 3) {
        return 1; // Successfully parsed all 3 parts
    }
    return 0; // Parsing failed
    }
    ```
3. Using AI for generating the function `int match_condition(Report *r, const char *field, const char *op, const char *value);` for the project:
   -  Promt used:
    ```txt
    I need a function with following signature:
    int match_condition(Report *r, const char *field, const char *op, const char *value);
    havinf the following Report struct {
        int severity;
        char category[24];
        char inspector[32];
    }
    
    Also:
    1. The function returns 1 if record satisfies the condition and 0 the other way
    2. severity is compared using all comparison operators
    3. category and inspector compared only with == and !=
    ```
   -  Answer generated:
    ```c
    int match_condition(Report *r, const char *field, const char *op, const char *value) {
        if (strcmp(field, "severity") == 0) {
            int target_val = atoi(value); // Convert string to integer 
            if (strcmp(op, "==") == 0) return r->severity == target_val;
            if (strcmp(op, "!=") == 0) return r->severity != target_val;
            if (strcmp(op, "<") == 0)  return r->severity < target_val;
            if (strcmp(op, "<=") == 0) return r->severity <= target_val;
            if (strcmp(op, ">") == 0)  return r->severity > target_val;
            if (strcmp(op, ">=") == 0) return r->severity >= target_val;
        } 
        else if (strcmp(field, "category") == 0) {
            // Strings only support equality/inequality
            if (strcmp(op, "==") == 0) return strcmp(r->category, value) == 0;
            if (strcmp(op, "!=") == 0) return strcmp(r->category, value) != 0;
        }
        else if (strcmp(field, "inspector") == 0) {
            if (strcmp(op, "==") == 0) return strcmp(r->inspector, value) == 0;
            if (strcmp(op, "!=") == 0) return strcmp(r->inspector, value) != 0;
        }
        
        // If the field or operator is unsupported, fail the match
        return 0;
    }

    ```

### Phase 2

4. Using AI to modify the makefile to compile both city_manager.c and monitor_reports.c for the project:
   - Promt used : 
    ```makefile
        How can i  modiffy  this  makefile  so i could compile all files including city_manager and monitor_report, which  both  have int main() : 
        CC = gcc
        CFLAGS = -Wall -Wextra -g

        SRCS = $(wildcard *.c)

        OBJS = $(SRCS:.c=.o)

        TARGET = city_manager

        all: $(TARGET)

        $(TARGET): $(OBJS)
            $(CC) $(CFLAGS) -o $@ $^

        %.o: %.c
            $(CC) $(CFLAGS) -c $< -o $@

        clean:
            rm -f $(TARGET) $(OBJS)
            rm -f active_reports-*

        .PHONY: all clean
    ```
   - Answer generated:
    ```makefile
    Important note: Makefiles strictly require Tabs for indentation, not spaces. When you paste this, make sure the indents under the rules are actual Tab characters.
    CC = gcc
    CFLAGS = -Wall -Wextra -g

    # Define your two target executables
    TARGET1 = city_manager
    TARGET2 = monitor_reports

    # Explicitly list the source files for each program
    # (Don't forget filter.c belongs to city_manager!)
    SRCS1 = city_manager.c filter.c
    SRCS2 = monitor_reports.c

    # Create object file variables for each
    OBJS1 = $(SRCS1:.c=.o)
    OBJS2 = $(SRCS2:.c=.o)

    # The default rule now builds BOTH programs
    all: $(TARGET1) $(TARGET2)

    # Rule to build city_manager
    $(TARGET1): $(OBJS1)
        $(CC) $(CFLAGS) -o $@ $^

    # Rule to build monitor_reports
    $(TARGET2): $(OBJS2)
        $(CC) $(CFLAGS) -o $@ $^

    # Universal rule to compile .c files into .o files
    %.o: %.c
        $(CC) $(CFLAGS) -c $< -o $@

    # Cleanup rule
    clean:
        rm -f $(TARGET1) $(TARGET2) $(OBJS1) $(OBJS2)
        rm -f active_reports-*
        rm -f .monitor_pid

    .PHONY: all clean
    ```
5.  Using AI to veriffy some missing if conddition so the project would be more robust. 
    - Promt used :
    ```txt
    now rewrite notify monitor without vulnerabilities:
    void notify_monitor(const char *district) {
        int notification_successful = 0;

        int pid_fd = open(".monitor_pid", O_RDONLY);
        if (pid_fd != -1) {
            char pid_str[32] = {0};
            if (read(pid_fd, pid_str, sizeof(pid_str) - 1) > 0) {
                pid_t monitor_pid = atoi(pid_str);

                if (monitor_pid > 0 && kill(monitor_pid, SIGUSR1) == 0) {
                    notification_successful = 1;
                }
            }
            close(pid_fd);
        }

        char log_path[FILE_PATH_SIZE];
        snprintf(log_path, sizeof(log_path), "%s/logged_district", district);

        int log_fd = open(log_path, O_WRONLY | O_APPEND);
        if (log_fd != -1) {
            char log_msg[128];
            int len;

            if (notification_successful) {
                len = snprintf(
                    log_msg, sizeof(log_msg),
                    "Event Notification: SUCCESS (Monitor informed via SIGUSR1)\n");
            } else {
                len = snprintf(
                    log_msg, sizeof(log_msg),
                    "Event Notification: FAILED (Monitor could not be informed)\n");
            }

            write(log_fd, log_msg, len);
            close(log_fd);
        } else {
            perror("System Warning: Could not open logged_district to append "
                "notification status");
        }
    }
    ```
    -  Answer generated:
    ``` makefile
    Here is the rewritten notify_monitor function with the vulnerabilities addressed.
    void notify_monitor(const char *district) {
        int notification_successful = 0;

        int pid_fd = open(".monitor_pid", O_RDONLY);
        if (pid_fd != -1) {
            char pid_str[32] = {0};
            ssize_t bytes_read = read(pid_fd, pid_str, sizeof(pid_str) - 1);
            
            if (bytes_read > 0) {
                // Explicitly null-terminate exactly at the end of what was read
                pid_str[bytes_read] = '\0';
                
                // Strip any trailing newline or carriage return for clean parsing
                pid_str[strcspn(pid_str, "\r\n")] = 0;

                // Use atol() for safer casting to pid_t (which can be a long)
                pid_t monitor_pid = (pid_t)atol(pid_str);

                if (monitor_pid > 0 && kill(monitor_pid, SIGUSR1) == 0) {
                    notification_successful = 1;
                }
            }
            close(pid_fd);
        }

        char log_path[FILE_PATH_SIZE];
        snprintf(log_path, sizeof(log_path), "%s/logged_district", district);

        int log_fd = open(log_path, O_WRONLY | O_APPEND);
        if (log_fd != -1) {
            char log_msg[128];
            int len;

            if (notification_successful) {
                len = snprintf(
                    log_msg, sizeof(log_msg),
                    "Event Notification: SUCCESS (Monitor informed via SIGUSR1)\n");
            } else {
                len = snprintf(
                    log_msg, sizeof(log_msg),
                    "Event Notification: FAILED (Monitor could not be informed)\n");
            }

            // VULNERABILITY FIX: Ensure the write succeeds fully
            if (write(log_fd, log_msg, len) != len) {
                perror("System Error: Failed to write notification status to logged_district");
            }
            
            // Bonus fix: Ensure the close succeeds
            if (close(log_fd) == -1) {
                perror("System Error: Failed to close logged_district");
            }
        } else {
            perror("System Warning: Could not open logged_district to append "
                "notification status");
        }
    }
    ```


### Phase 3

   6. Used  AI to generate the pseudocode for  the scorer, to  understand better the  structure of the cod
     - Promt used:
     ```txt
        Act as a senior systems programming instructor. Please generate detailed, structured pseudocode for a standalone C program called scorer.

        Program Objective: > Read a binary data file containing municipal reports for a specific city district, calculate the total workload score (sum of severity levels) for each unique inspector, and print a formatted summary to standard output.

        Context & Constraints:

            Input: The program takes exactly one command-line argument: the district_id (a string).

            File System: The data is stored in a binary file located at <district_id>/reports.dat.

            Data Structure: The file is populated sequentially with Report structs. Assume the struct contains at least:

                inspector (string: name of the inspector)

                severity (integer: a value usually between 1 and 3)

            Logic Requirements:

                Safely check if the correct number of arguments is provided.

                Attempt to open the binary file for reading. If it fails, print a safe error message and exit cleanly (do not crash).

                Initialize a data structure (like an array of records) to keep track of unique inspectors and their running total of severity points.

                Loop through the binary file, reading one Report struct at a time until EOF.

                For each record, check if the inspector is already in the tracking array. If yes, add the severity to their total. If no, add them as a new entry with their initial severity.

                Close the file.

            Output Requirements:

                Loop through the tracking array and print a formatted summary showing each inspector's name and their total workload score.

        Please write the pseudocode clearly, using standard algorithmic control structures (IF/THEN, WHILE, FOR) and abstracting away pure C-specific syntax (like open() or read()) into readable actions.
     ```

     - Answer generated: 
    ```txt
        // ---------------------------------------------------------
        // DATA STRUCTURE DEFINITIONS
        // ---------------------------------------------------------

        // Define the structure as it is packed in the binary file
        STRUCT Report
            STRING inspector
            INTEGER severity
        END STRUCT

        // Define a structure to keep track of our running totals in memory
        STRUCT InspectorScore
            STRING name
            INTEGER total_score
        END STRUCT

        // ---------------------------------------------------------
        // MAIN PROGRAM EXECUTION
        // ---------------------------------------------------------

        FUNCTION Main(arguments)

            // 1. Validate Command-Line Arguments
            // Expecting exactly 2 arguments: the program name itself, and the district_id
            IF length of arguments != 2 THEN
                PRINT "Usage: scorer <district_id>"
                EXIT with status code 1 (Failure)
            END IF

            SET district_id = arguments[1]

            // 2. Construct the File Path
            // Concatenate the strings to form the target path
            SET file_path = district_id + "/reports.dat"

            // 3. Safe File Handling
            OPEN file at file_path in BINARY READ mode AS file_ptr
            
            IF file_ptr is NULL THEN
                PRINT "Error: Could not open data file at " + file_path
                EXIT with status code 2 (Failure)
            END IF

            // 4. Initialize Tracking Data Structure
            // For simplicity in pseudocode, we use a fixed-size array. 
            // In production C, consider a hash map or dynamic array (malloc/realloc).
            CONSTANT MAX_INSPECTORS = 100
            ARRAY scores OF InspectorScore[MAX_INSPECTORS]
            SET unique_inspectors_count = 0

            // 5. Data Processing Loop
            DECLARE current_report AS Report

            // Loop continues as long as we successfully read one full Report struct
            WHILE READ_BINARY_STRUCT(file_ptr, current_report) is SUCCESSFUL DO
                SET found = FALSE

                // Linear search to see if we already track this inspector
                FOR i = 0 TO unique_inspectors_count - 1 DO
                    IF scores[i].name EQUALS current_report.inspector THEN
                        // Inspector found: Add the severity to their running total
                        scores[i].total_score = scores[i].total_score + current_report.severity
                        SET found = TRUE
                        BREAK FOR loop // Stop searching once found
                    END IF
                END FOR

                // 6. Handle New Entries
                IF found is FALSE THEN
                    // Boundary check to prevent buffer overflow
                    IF unique_inspectors_count < MAX_INSPECTORS THEN
                        scores[unique_inspectors_count].name = current_report.inspector
                        scores[unique_inspectors_count].total_score = current_report.severity
                        unique_inspectors_count = unique_inspectors_count + 1
                    ELSE
                        PRINT "Warning: Maximum inspector limit (" + MAX_INSPECTORS + ") reached."
                        PRINT "Skipping new inspector: " + current_report.inspector
                        // Continuing to process existing inspectors
                    END IF
                END IF

            END WHILE

            // 7. Cleanup Resources
            CLOSE file_ptr

            // 8. Formatted Output
            PRINT "========================================"
            PRINT "Workload Summary for District: " + district_id
            PRINT "========================================"
            
            FOR i = 0 TO unique_inspectors_count - 1 DO
                PRINT "Inspector: " + scores[i].name + " | Total Score: " + scores[i].total_score
            END FOR

            EXIT with status code 0 (Success)

        END FUNCTION
    ```

    7. I  used AI to  explain where  and  how should be used the function close(fd[1])
        - Promt used:
        ```txt
        Act as an expert Unix systems programming instructor. I am writing a C program (a "hub") that needs to spawn multiple child processes in a loop. For each child, I create a pipe(), fork(), and use dup2() in the child to redirect its stdout to the write-end of the pipe.

        In the parent process, I save the read-ends of all these pipes into an array. After the spawning loop finishes, the parent loops through that array, uses fdopen() to convert the file descriptors, and calls fgets() to read the output from each child until the pipe closes.

        The Problem: > My parent process is getting stuck. The fgets() function hangs infinitely waiting for data, even after the child process has seemingly finished executing its task. I suspect this is a deadlock caused by open file descriptors.

        What I need you to generate:

            The "Why": Explain exactly why failing to close the write-end of the pipe (fd[1]) in the parent process causes fgets() to hang. Explain how the OS determines when an End-Of-File (EOF) is reached on a pipe.

            The "Where" and "How": Show me exactly where close(fd[1]) needs to be placed within my code structure to fix this.

            Code Snippet: Provide a minimal, clear C code snippet of the for loop that does the pipe() and fork(), highlighting the exact line where the parent must close its copy of the write-end.
        ```

        - Answer generated:
        ```txt
            The "Why": Pipe Reference Counting and EOF

        To understand why fgets() hangs, we must look at how the operating system manages pipes and file descriptors under the hood.

        When you invoke pipe(fd), the OS creates a unidirectional data channel in kernel memory and returns two file descriptors: fd[0] (read) and fd[1] (write).

        When you subsequently call fork(), the child process inherits exact copies of all the parent's open file descriptors. Therefore, immediately after a fork, both the parent and the child have an open fd[0] and an open fd[1] pointing to the same underlying kernel pipe.

        The EOF Rule for Pipes:
        A read operation (like read() or fgets()) on a pipe will only return an End-Of-File (EOF) marker when the OS sees that all write-ends of that pipe have been closed. The kernel maintains a reference count for the write-end.

        Your Deadlock Scenario:

            You pipe and fork. Both parent and child hold fd[1].

            The child redirects its stdout, writes its data, and exits. When the child exits, the OS automatically closes the child's copy of fd[1].

            However, the parent process still holds its own original copy of fd[1].

            The parent enters a loop and calls fgets() on the read-end. fgets() consumes all the data the child wrote.

            Once the data buffer is empty, fgets() asks the kernel: "Is there any more data coming?"

            The kernel checks the reference count for fd[1]. Because the parent still has fd[1] open, the kernel assumes someone (the parent itself) might write to the pipe in the future. It does not send an EOF.

            fgets() blocks infinitely, waiting for data that will never arrive.

        The "Where" and "How": Placing the Close

        To fix this, you must explicitly sever the parent's connection to the write-end of the pipe.

        The exact place to do this is inside your spawning loop, in the parent's execution branch, immediately after the fork() succeeds and you have saved the read-end. The parent will never write to this pipe, so it has no business keeping fd[1] open.

        By closing it immediately, you ensure that the only process holding an open fd[1] is the child. When the child finishes and exits, the reference count drops to zero, the kernel signals EOF, and your parent's fgets() loop will unblock and terminate gracefully.
        The Code Snippet

        A Note on Child Process Hygiene: Notice that in the child block, I also closed fd[0], and after calling dup2(), I closed the original fd[1]. While failing to do this won't usually cause a deadlock like the parent's fd[1] does, leaving unnecessary descriptors open in the child is poor practice and can lead to descriptor exhaustion in larger, more complex systems. Always close what you do not use.
        ```
        ```c
            #include <stdio.h>
            #include <stdlib.h>
            #include <unistd.h>
            #include <sys/wait.h>

            #define NUM_CHILDREN 5

            int main() {
                int read_fds[NUM_CHILDREN];
            pid_t pids[NUM_CHILDREN];

            // 1. Spawning Loop
            for (int i = 0; i < NUM_CHILDREN; i++) {
                int fd[2];
                
                if (pipe(fd) == -1) {
                    perror("pipe failed");
                    exit(EXIT_FAILURE);
                }

                pid_t pid = fork();

                if (pid < 0) {
                    perror("fork failed");
                    exit(EXIT_FAILURE);
                }

                if (pid == 0) {
                    // ==========================================
                    // CHILD PROCESS
                    // ==========================================
                    close(fd[0]); // Child doesn't read from this pipe

                    // Redirect stdout to the pipe's write-end
                    if (dup2(fd[1], STDOUT_FILENO) == -1) {
                        perror("dup2 failed");
                        exit(EXIT_FAILURE);
                    }
                    close(fd[1]); // Close the original fd after duplication

                    // Execute task (simulated with printf)
                    printf("Child %d reporting status: SUCCESS\n", i);
                    exit(EXIT_SUCCESS);
                } else {
                    // ==========================================
                    // PARENT PROCESS
                    // ==========================================
                    pids[i] = pid;
                    read_fds[i] = fd[0]; // Save the read-end for later

                    // *** THE CRITICAL FIX ***
                    // The parent must close its copy of the write-end immediately.
                    // If this is omitted, subsequent reads will deadlock.
                    close(fd[1]); 
                }
            }

            // 2. Reading Loop (After all children are spawned)
            for (int i = 0; i < NUM_CHILDREN; i++) {
                FILE *stream = fdopen(read_fds[i], "r");
                if (stream == NULL) {
                    perror("fdopen failed");
                    continue;
                }

                char buffer[256];
                // Because the parent closed fd[1], this will cleanly hit EOF 
                // when the child exits.
                while (fgets(buffer, sizeof(buffer), stream) != NULL) {
                    printf("Parent received: %s", buffer);
                }

                fclose(stream); // This also closes the underlying read_fds[i]
            }

            // 3. Cleanup: Wait for all children to prevent zombies
            for (int i = 0; i < NUM_CHILDREN; i++) {
                waitpid(pids[i], NULL, 0);
            }

            return 0;
            }
        ```
    8. I used AI to give me an example of printf from monitor_reports.c so the city_hub.c would work  the right way.
       - Promt used: 
        ```txt
            Act as a senior C systems programmer. I am building a project where a parent process (city_hub.c) reads the output of a background child process (monitor_reports.c) in real-time through a pipe using fgets().

            The Goal: > I need to structure the printf statements in monitor_reports.c using strict prefixes so that city_hub.c can easily parse the strings, categorize the messages, and display custom alerts to the user.

            What I need you to generate:

                Monitor Formatting: Provide 4 distinct examples of printf statements for monitor_reports.c covering these scenarios:

                    Startup Information (e.g., printing its PID).

                    An Event Triggered (e.g., a new report was added).

                    An Error (e.g., another monitor is already running).

                    A clean Shutdown message.
                    (Please use a clear delimiter like "MSG:TYPE:" in the strings).

                Hub Parsing Logic: Provide a brief C code snippet showing how the while(fgets(...)) loop in city_hub.c would parse these specific prefixes (using strncmp or sscanf) to strip the prefix and print a clean, custom-formatted message to the user's terminal.

                The Buffer Trap: Briefly remind me what function I must call at the start of monitor_reports.c to ensure printf flushes to the pipe immediately, rather than getting stuck in the C standard library buffer.
        ```

        - Answer generated:
        ```txt
             1. Monitor Formatting (monitor_reports.c)

         We will use the strict prefix format MSG:<TYPE>: to make parsing deterministic. Note the inclusion of the newline \n at the end of each statement; this is mandatory for fgets() in the parent process to know when a message ends.
         C

         // 1. Startup Information
         printf("MSG:START:Monitor initialized with PID %d.\n", getpid());

         // 2. An Event Triggered
         printf("MSG:EVENT:New report added in district_id 7.\n");

         // 3. An Error
         // Note: Errors often go to stderr, but if you are piping stdout to the hub, 
         // you must ensure the hub sees it by sending it over the same piped stream.
         printf("MSG:ERROR:Initialization failed. Another monitor instance is already running.\n");

         // 4. A Clean Shutdown
         printf("MSG:SHUTDOWN:Teardown complete. Monitor exiting safely.\n");

         1. Hub Parsing Logic (city_hub.c)

         When parsing with fgets(), using strncmp() is vastly more efficient and safer than sscanf() for simple prefix matching.

         Critical Systems Note: fgets() retains the trailing newline character (\n) in the buffer. When rewriting the output for the user, you must either account for it or strip it. In this example, I will show you how to safely advance the string pointer past the prefix to print the payload.
         C

         #include <stdio.h>
         #include <string.h>

         // ... inside your parent process ...

         char buffer[512];
         // Assuming pipe_fp is the FILE* created via fdopen(fd[0], "r")

         while (fgets(buffer, sizeof(buffer), pipe_fp) != NULL) {
             
             // Optional but highly recommended: Strip the trailing newline left by fgets
             buffer[strcspn(buffer, "\n")] = '\0'; 

             // Parse the prefixes using strncmp
             if (strncmp(buffer, "MSG:START:", 10) == 0) {
                 // Advance the pointer by 10 characters to skip the prefix
                 char *payload = buffer + 10;
                 printf("[\033[1;32mSYSTEM\033[0m] %s\n", payload); // Green text for start
                 
             } 
             else if (strncmp(buffer, "MSG:EVENT:", 10) == 0) {
                 char *payload = buffer + 10;
                 printf("[\033[1;34mINFO\033[0m] %s\n", payload);   // Blue text for events
                 
             } 
             else if (strncmp(buffer, "MSG:ERROR:", 10) == 0) {
                 char *payload = buffer + 10;
                 printf("[\033[1;31mALERT\033[0m] %s\n", payload);  // Red text for errors
                 
             } 
             else if (strncmp(buffer, "MSG:SHUTDOWN:", 13) == 0) {
                 char *payload = buffer + 13;
                 printf("[\033[1;33mSTOP\033[0m] %s\n", payload);   // Yellow text for shutdown
                 
             } 
             else {
                 // Fallback for unformatted strings or unexpected garbage
                 printf("[UNKNOWN] %s\n", buffer);
             }
         }

         2.  The Buffer Trap

         The function you absolutely must call at the very beginning of monitor_reports.c is:
         C

         setvbuf(stdout, NULL, _IONBF, 0);

         Why this is critical:
         By default, the C Standard Library line-buffers stdout only if it detects that it is connected to an interactive terminal (a TTY).

         When you connect monitor_reports.c to a pipe, the OS tells the C library that stdout is now a block device. The library automatically switches from line-buffered to block-buffered. It will silently hold onto all your printf output until an internal 4KB or 8KB buffer is completely full, causing the fgets() in your hub to hang waiting for data that hasn't been flushed.

         Calling setvbuf with _IONBF (I/O No Buffering) disables this behavior entirely, forcing the OS to push the string through the pipe the absolute millisecond printf is called. Alternatively, you could call fflush(stdout); after every single printf, but setting setvbuf at startup is the bulletproof, set-it-and-forget-it solution.
        ```
        
    9.  I used AI for debugging the code and to  explain some of the error that appeared durring the implementation of the code in all the phases  so I would not make a misstake that will affect my  computter especialy for the handle_remove_district function. 

## What I changed

1. I have added to the makefile the line :  `rm -f active_reports-*` to clean up any old report files from previous runs without throwing an error if no files exist.
2. I have added the null pointer verification for all function  parameters in match_condition `if (!r || !field || !op || !value) return 0;`
3. I have added  `if (input == NULL || field == NULL || op == NULL || value == NULL) {
        return 0;
    }` to  check for null pointers.
4. I have added in the function match_condition the missing timestamp field in the filter logic :   
    ```c
    else if (strcmp(field, "timestamp") == 0) {
        time_t target_val = (time_t)atol(value);
        if (strcmp(op, "==") == 0) return r->timestamp == target_val;
        if (strcmp(op, "!=") == 0) return r->timestamp != target_val;
        if (strcmp(op, "<") == 0)  return r->timestamp < target_val;
        if (strcmp(op, "<=") == 0) return r->timestamp <= target_val;
        if (strcmp(op, ">") == 0)  return r->timestamp > target_val;
        if (strcmp(op, ">=") == 0) return r->timestamp >= target_val;
    }
    ```
5. I have written the scorrer.c based on the pseudocode and added the city_manager.h  for a better and saffer  performance.
6. About the `close(fd[1])`  I had  used  the example to implement based  on the project and all the specification given.
7. I used the example of  printf and I  modified the monitor_reports to  work the proper way.
8. I used the explanation for the errors to correct my mistakes.


## What I have learned  

1. I learned how to use `sscanf` with custom delimiter sets (`%[^:]`) as a highly efficient alternative to manual string splitting with `strtok`.
2. I learned that the Unix shell interprets `<` and `>` as I/O redirection operators before the C program ever sees them. To pass these characters into `argv`, I learned they must be escaped with a backslash (`severity:\<:3`) or enclosed in quotes.
3. I learned that I have to specify all the details for a prompt in order for AI to give a proper answer.
4. I learned how to properly write a makefile. 
5. I learned that we can't use wildcards in makefile to generate 2 executables.
6. I learned in a shorter way how pid and pid_t works.
7. I learned how and where should be used `close(fd[1])`.
8. I learned how Processes, Signals, Pipes and Redirects work and how  should I use different specific functions.
9. I learned the proper way of writing a cod  so it would work in parralel.
10. I learned more about  different  functions from C. 
