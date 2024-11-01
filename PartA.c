#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
// #include <readline/readline.h>
// #include <readline/history.h>

int bg = 0;

void run(char *input)
{
    char *args[500];
    char *token = strtok(input, " ");
    int count = 0;
    while (token != NULL)
    {
        args[count++] = token;
        token = strtok(NULL, " ");
    }
    args[count] = NULL;

    if (!strcmp(args[0], "pwd"))
    {
        if (execlp("pwd", "pwd", NULL) == -1)
        {
            perror("execlp pwd failed");
            exit(1);
        }
    }
    else if (!strcmp(args[0], "cd"))
    {
        if (count != 2)
        {
            fprintf(stderr, "Usage: cd <directory>\n");
        }
        else
        {
            if (chdir(args[1]) != 0)
            {
                perror("cd");
            }
        }
    }
    else if (!strcmp(args[0], "mkdir"))
    {
        if (count != 2)
        {
            fprintf(stderr, "Usage: mkdir <directory_name>\n");
        }
        else
        {
            if (mkdir(args[1], 0755) != 0)
            {
                perror("mkdir");
            }
        }
    }
    else if (!strcmp(args[0], "ls"))
    {
        if (count > 0)
        {
            execvp("ls", args);
        }
        else
        {
            execlp("ls", "ls", NULL);
        }
    }
    else if (!strcmp(args[0], "help"))
    {
        printf("Available commands:\n");
        printf("1. pwd - shows the present working directory\n");
        printf("2. cd <directory_name> - changes the present working directory\n");
        printf("3. mkdir <directory_name> - creates a new directory\n");
        printf("4. ls <flag> - shows the contents of a directory\n");
        printf("5. exit - exits the shell\n");
        printf("6. help - prints this list of commands\n");
        printf("7. Execute any other command\n");
    }
    else
    {
        printf("Command not recognized. Type 'help' for a list of commands.\n");
    }
    exit(0);
}

void exec(char *input)
{
    pid_t child_pid;
    child_pid = fork(); // Create a child process
    int status;

    if (child_pid == -1)
    {
        perror("Fork/process creation failed");
        exit(1);
    }
    else if (child_pid == 0)
    {
        run(input);
    }
    else
    {
        // printf("");
        if (bg)
        {
            bg = 0;
            return;
        }
        else
        {
            wait(&status);
            waitpid(child_pid, &status, 0);

            if (WIFEXITED(status))
            {
            }
        }
    }
}

int main()
{
    char input[500];
    int i = 0;

    while (1)
    {
        printf("shell> ");
        fgets(input, sizeof(input), stdin);
        if (!strcmp(input, "exit\n") || !strcmp(input, "exit&\n") || !strcmp(input, "exit &\n"))
        {
            // printf("1");
            return 0;
        }
        if (input[strlen(input) - 2] == '&')
        {
            bg = 1;
            input[strlen(input) - 2] = '\0';
        }
        else
        {
            input[strlen(input) - 1] = '\0';
        }
        exec(input);
        bg = 0;
    }
    return 0;
}