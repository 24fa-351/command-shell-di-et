#ifndef COMMAND_H
#define COMMAND_H

typedef char *Term;

typedef struct command
{
    Term *terms;

    Term raw_command;

    Term input_file;  // <
    Term output_file; // >> or >
    bool append;      // >>

    int input_fd;
    int output_fd;

    bool background;   // &
} Command;

// allocates a command structure off the heap
Command *read_command();

void execute_command(Command *command);

void free_command(Command *command);

void print_command(Command *command);

#endif
