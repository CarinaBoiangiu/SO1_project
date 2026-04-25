# AI usage

## What The AI was used for

1. The AI was used for generating the Makefile for the project
    - Prompt utilizat

    ```txt
    Make me a makefile for a c project
    use wildcards so there won't be needed to add more file definitions in the future
    all files will be located in the same directory
    target is named city_manager
    add clean for objs and target
    ```

    - Raspuns generat:

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