#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

#include "command.h"

int main(int argc, char *argv[])
{
    while (1)
    {
        printf("$>");

        Command *command = read_command();
        if (command == NULL)
        {
            printf("EOF\n");
            exit(0);
        }
        

        execute_command(command);
        
        print_command(command);

        free_command(command);
    }
    return 0;
}