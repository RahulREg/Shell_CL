#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <readline/readline.h>
#include <readline/history.h>

int findPipes(char *input)
{
    int c = 0;
    for (int i = 0; i < strlen(input); i++)
    {
        if (input[i] == '|')
            c++;
    }
    // printf("%d\n", c);
    return c;
}

void execute_piped_commands(char *input)
{
    char *command;
    int cmd_len = findPipes(input);
    command = strtok(input, "|");

    int i, j = 0, k = 0;
    cmd_len++;
    pid_t pid;
    int fd[2 * cmd_len];

    // pipe(2) for cmd_len times
    for (i = 0; i < cmd_len; i++)
    {
        if (pipe(fd + i * 2) < 0)
        {
            perror("couldn't pipe");
            exit(EXIT_FAILURE);
        }
    }
    while (command != NULL)
    {
        if ((pid = fork()) == -1)
        {
            perror("fork");
            exit(1);
        }
        else if (pid == 0)
        {
            // printf("%d %d\n", k, cmd_len);
            if (k != cmd_len - 1) // if there is next
            {
                if (dup2(fd[j + 1], 1) < 0)
                {
                    perror("dup21");
                    exit(EXIT_FAILURE);
                }
            }
            // if there is previous
            if (j != 0)
            {
                if (dup2(fd[j - 2], 0) < 0)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            for (i = 0; i < 2 * cmd_len; i++)
            {
                close(fd[i]);
            }

            char *args[500];
            char *token = strtok(command, " ");
            int arg_count = 0;

            while (token != NULL && arg_count < 500 - 1)
            {
                args[arg_count++] = token;
                token = strtok(NULL, " ");
            }
            args[arg_count] = NULL;

            if (execvp(args[0], args) < 0)
            {
                perror((args)[0]);
                exit(EXIT_FAILURE);
            }
        }
        else if (pid < 0)
        {
            perror("error");
            exit(EXIT_FAILURE);
        }

        // no wait in each process,
        // because I want children to exec without waiting for each other
        // as bash does.
        command = strtok(NULL, "|");
        j += 2;
        k++;
    }
    // close fds in parent process
    for (i = 0; i < 2 * cmd_len; i++)
    {
        close(fd[i]);
    }
    // wait for children
    for (i = 0; i < cmd_len; i++)
        wait(NULL);
}

int isPipe(char *input)
{
    int found = 0; // Flag to indicate if the '|' character is found

    for (int i = 0; input[i] != '\0'; i++)
    {
        if (input[i] == '|')
        {
            found = 1;
            break; // Exit the loop as soon as '|' is found
        }
    }
    return found;
}

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

    if (!strcmp(args[0], "help"))
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
    else if (!strcmp(args[0], "cd"))
    {
    }
    else if (!strcmp(args[0], "vi"))
    {
        if(execlp("./editor.out", "./editor.out", args[1], NULL) == -1) {
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // printf("Command not recognized. Type 'help' for a list of commands.\n");
        if (execvp(args[0], args) == -1)
        {
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
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
    char input[500] = "";
    char *input_line;
    using_history();

    while (1)
    {
        input_line = readline("shell> ");
        // printf("%s", input_line);
        if (input_line == NULL || !strcmp(input_line, ""))
        {
            free(input_line);
            continue;
        }
        if (input_line[strlen(input_line) - 1] == '\\')
        {
            // Handle multi-line input
            input_line[strlen(input_line) - 1] = '\0';
            strcat(input, input_line);
            free(input_line);
            continue;
        }
        else
        {
            strcat(input, input_line);
            free(input_line);

            // Add the command to history
            add_history(input);

            if (isPipe(input))
            {
                execute_piped_commands(input);
            }
            else
            {
                if (input[strlen(input) - 1] == '&')
                {
                    bg = 1;
                    input[strlen(input) - 1] = '\0';
                }
                if (!strcmp(input, "exit") || !strcmp(input, "exit&") || !strcmp(input, "exit &"))
                {
                    break;
                }
                exec(input);
                char *args[500];
                char *token = strtok(input, " ");
                int count = 0;
                while (token != NULL)
                {
                    args[count++] = token;
                    token = strtok(NULL, " ");
                }
                args[count] = NULL;
                if (!strcmp(args[0], "cd"))
                {
                    if (chdir(args[1]) != 0)
                    {
                        perror("cd");
                    }
                }
                bg = 0;
            }

            input[0] = '\0'; // Reset the input buffer
        }
    }

    return 0;
}