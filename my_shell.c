#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_LINE 1000

int main(int argc, char *argv[])
{

    char line[MAX_LINE];
    while (1)
    {
        printf("$>");
        fgets(line, MAX_LINE, stdin);
        if (fgets(line, MAX_LINE, stdin) == NULL)
        {
            perror("'fgets' returned NULL");
            break;
        }
        line[strlen(line) - 1] = '\0'; // remove the newline character at the end of the line so that we can compare the string

        if (strcmp(line, "exit") == 0) // if the user enters "exit", the program will exit
        {
            break;
        }
        printf("you entered '%s'\n", line);

        char *args[MAX_LINE];
        char *token = strtok(line, " "); // strtok() is used to split the string into tokens, line is the string to be split, " " is the delimiter
        int i = 0;                       //

        while (token != NULL)
        {
            args[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        printf("args[0] = %s\n", args[0]);

        args[i] = NULL;

        pid_t pid = fork();
        if (pid == 0)
        {
            execvp(args[0], args);
            perror("execvp");
            exit(1);
        }
        else
        {
            wait(NULL);
        }
    }
    return 0;
}

/*

*/