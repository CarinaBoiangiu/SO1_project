# AI usage

## The Tool used

-  Google gemini


## What The AI was used for

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

## What I changed

1.  I have added to the makefile the line :  `rm -f active_reports-*` to clean up any old report files from previous runs without throwing an error if no files exist.
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


## What I have learned  

1. I learned how to use `sscanf` with custom delimiter sets (`%[^:]`) as a highly efficient alternative to manual string splitting with `strtok`.
2. I learned that the Unix shell interprets `<` and `>` as I/O redirection operators before the C program ever sees them. To pass these characters into `argv`, I learned they must be escaped with a backslash (`severity:\<:3`) or enclosed in quotes.
3. I learned that I have to specify all the details for a prompt in order for AI to give a proper answer.
4. I learned how to properly write a makefile. 
5. I learned that we can't use wildcards in makefile to generate 2 executables.
6. I learned in a shorter way how pid and pid_t works.

