#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>

#include "command.h"

#define MAX_LINE 1000

#define CAN_EXECUTE(PATH) (PATH != NULL && access(PATH, X_OK) == 0)

void add_character_to_string(char *str, char c)
{
    int len = strlen(str);
    str[len] = c;
    str[len + 1] = '\0';
}

void split(char *cmd, char *words[], char delimiter)
{
    int word_count = 0;
    char *next_char = cmd;
    char current_word[1000];
    strcpy(current_word, "");

    while (*next_char != '\0')
    {
        if (*next_char == delimiter)
        {
            words[word_count++] = strdup(current_word);
            strcpy(current_word, "");
        }
        else
        {
            add_character_to_string(current_word, *next_char);
        }
        ++next_char;
    }
    words[word_count++] = strdup(current_word);
    words[word_count] = NULL;
}

// the return value is the allocated path of the command or NULL.
char *find_absolute_path(char *cmd)
{
    printf("finding absolute path for: '%s'\n", cmd);

    char *directories[1000];

    split(getenv("PATH"), directories, ':');

    for (int ix = 0; directories[ix] != NULL; ix++)
    {
        char path[1000];

        printf("checking directory: %s\n", directories[ix]);
        strcpy(path, directories[ix]);
        add_character_to_string(path, '/');
        strcat(path, cmd);
        printf("checking path: %s\n", path);

        if (CAN_EXECUTE(path))
        {
            return strdup(path);
        }
    }

    for (int iy = 0; directories[iy] != NULL; iy++)
    {
        free(directories[iy]);
    }

    return NULL;
}

char *read_line()
{
    char *line = malloc(MAX_LINE);
    int len_read = 0;
    while (1)
    {
        char ch;
        int number_bytes_read = read(STDIN_FILENO, &ch, 1); // read from the standard input
        if (number_bytes_read <= 0)
        {
            return NULL;
        }
        if (ch == '\n')
        {
            break;
        }
        line[len_read] = ch;
        len_read++;
        line[len_read] = '\0';
    }
    if (len_read > 0 && line[len_read - 1] == '\r')
    {
        line[len_read - 1] = '\0';
    }
    line = realloc(line, len_read + 1);
    return line;
}

Command *read_command()
{
    Command *command = malloc(sizeof(Command));

    command->raw_command = malloc(MAX_LINE); // (*command).raw_command = malloc(MAX_LINE); - attribute
    command->raw_command[0] = '\0';          // raw_command is a NULL terminated string

    command->terms = malloc(MAX_LINE * sizeof(Term));
    command->terms[0] = NULL;

    command->input_file = NULL;
    command->output_file = NULL;
    command->append = false;
    command->input_fd = -1;
    command->output_fd = -1;
    command->background = false;

    int number_of_raw_characters_read = 0;
    int number_of_terms_read = 0;

    char current_term[1000];
    current_term[0] = '\0';

    while (1)
    {
        int ch;
        ch = fgetc(stdin);

        if (ch == EOF)
        {
            return NULL;
        }
        if (ch == '\n')
        {
            break;
        }

        if (ch == ' ')
        {
            if (current_term[0] != '\0')
            {
                command->terms[number_of_terms_read] = strdup(current_term);
                number_of_terms_read++;
                command->terms[number_of_terms_read] = NULL;
                current_term[0] = '\0';
            }
        }
        else
        {
            add_character_to_string(current_term, ch);
        }

        command->raw_command[number_of_raw_characters_read] = ch; // add the character to the raw_command
        number_of_raw_characters_read++;
        command->raw_command[number_of_raw_characters_read] = '\0'; // add the null terminator
    }
    if (current_term[0] != '\0')
    {
        command->terms[number_of_terms_read] = strdup(current_term);
        number_of_terms_read++;
        command->terms[number_of_terms_read] = NULL;
    }

    command->raw_command = realloc(command->raw_command, number_of_raw_characters_read + 1);
    command->terms = realloc(command->terms, number_of_terms_read + 1);

    return command;
}

// TODO: implement handle_internal_command

void execute_command(Command *command)
{
    if (strcmp(command->terms[0], "exit") == 0)
    {
        printf("exiting shell...\n");
        exit(0);
    }

    if (strcmp(command->terms[0], "pwd") == 0)
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("Current working dir: %s\n", cwd);
        }
        else
        {
            perror("getcwd() error");
        }
        return;
    }

    if (strcmp(command->terms[0], "cd") == 0)
    {
        if (chdir(command->terms[1]) == -1)
        {
            perror("chdir");
        }
        return;
    }

    // hard coded values for testing
    command->input_file = strdup("input.txt");

    // for it not to be hard coded, it should be like this:
    // command->input_file = strdup("input.txt");

    command->output_file = strdup("output.txt");

    char *executable_path = NULL;
    // printf("\nexecuting command raw_command '%s'\n", command->raw_command);

    if (command->terms[0][0] == '/' || command->terms[0][0] == '.')
    {
        printf("executing command: %s\n", command->terms[0]);
        executable_path = strdup(command->terms[0]);
    }
    else
    {
        executable_path = find_absolute_path(command->terms[0]); 
    }

    if (!CAN_EXECUTE(executable_path))
    {
        printf("command not found\n");
        return;
    }

    printf("executing command: %s\n", command->terms[0]);

    if (command->input_file != NULL)
    {
        command->input_fd = open(command->input_file, O_RDONLY);
        if (command->input_fd == -1)
        {
            perror("open input file");
            return;
        }
    }

    if (command->output_file != NULL)
    {
        int flags = O_WRONLY | O_CREAT | (command->append ? O_APPEND : O_TRUNC);

        command->output_fd = open(command->output_file, flags, 0644);

        if (command->output_fd == -1)
        {
            perror("open output file");
            return;
        }
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        if (command->input_fd != -1)
        {
            dup2(command->input_fd, STDIN_FILENO);
            close(command->input_fd);
        }

        if (command->output_fd != -1)
        {
            dup2(command->output_fd, STDOUT_FILENO);
            close(command->output_fd);
        }

        execve(executable_path, command->terms, NULL);
        perror("execve failed");
        exit(1);
    }
    else // if the process is the parent
    {
        int wait_status;

        if (command->input_fd != -1)
        {
            close(command->input_fd);
        }

        if (command->output_fd != -1)
        {
            close(command->output_fd);
        }

        waitpid(pid, &wait_status, 0); // wait for the child process to finish
    }
    free(executable_path);
}

void free_command(Command *command)
{
    if (command == NULL)
    {
        return;
    }
    if (command->terms != NULL)
    {
        for (int i = 0; command->terms[i] != NULL; i++)
        {
            free(command->terms[i]);
        }
        free(command->terms);
    }
    if (command->input_file != NULL)
    {
        free(command->input_file);
    }
    if (command->output_file != NULL)
    {
        free(command->output_file);
    }
    if (command->raw_command != NULL)
    {
        free(command->raw_command);
    }
    free(command);
}

void print_command(Command *command)
{
    if (command == NULL)
    {
        return;
    }
    if (command->terms != NULL)
    {
        for (int ix = 0; command->terms[ix] != NULL; ix++)
        {
            printf("term[%d]: '%s'\n", ix, command->terms[ix]);
        }
    }
    if (command->input_file != NULL)
    {
        printf("input file: '%s'\n", command->input_file);
    }
    if (command->output_file != NULL)
    {
        printf("output file: '%s'\n", command->output_file);
    }
    if (command->raw_command != NULL)
    {
        printf("raw_command: '%s'\n", command->raw_command);
    }

    printf("append: '%d'\n", command->append);
    printf("input_fd: '%d'\n", command->input_fd);
    printf("output_fd: '%d'\n", command->output_fd);
    printf("background: '%d'\n", command->background);
}
