#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SIZE 500

// int count_words(const char *str1, const char *str2)
// {
//     int count = 0;
//     char *temp1 = strdup(str1); // Create a copy since strtok() modifies the string
//     char *temp2 = strdup(str1);
//     char *token1 = strtok(temp1, " \t\r");
//     char *token2 = strtok(temp2, " \t\r");
//     while (token1 && token2)
//     {
//         if (strcmp(token1, token2) != 0)
//         {
//             count++;
//         }
//         token1 = strtok(NULL, " \t\r");
//         token2 = strtok(NULL, " \t\r");
//     }
//     while (token2)
//     {
//         count++;
//         token2 = strtok(NULL, " \t\r");
//     }
//     free(temp1);
//     free(temp2);
//     return count;
// }

void vi_editor(char *fname)
{
    int y = 0, x = 0; // y is current line number, x is current column
    int ch;           // to store user input
    int max_y = 0, max_x = 0, rows = 0, flag = 1;
    char buffer[SIZE][SIZE];
    char ini[SIZE][SIZE];

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    // start_color(); // Enable color support
    // init_pair(1, COLOR_RED, COLOR_BLACK);
    // attron(COLOR_PAIR(1));
    FILE *file = fopen(fname, "r");
    if (file == NULL)
    {
        return; // File not found or couldn't be opened
    }

    char line[SIZE];
    while (fgets(line, SIZE, file) != NULL && y < SIZE)
    {
        strcpy(buffer[y], line);
        buffer[y][strlen(buffer[y]) - 1] = '\0';
        y++;
    }
    fclose(file);
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            ini[i][j] = buffer[i][j];
        }
    }

    for (int i = 0; i < y; i++)
    {

        for (int j = 0; j < strlen(buffer[i]); j++)
        {
            if (buffer[i][j] != '\0' || buffer[i][j] != '\n')
            {
                mvchgat(i, j, 1, A_REVERSE, 0, NULL);
                mvprintw(i, j, "%s", buffer[i]);
                refresh();
            }
            else
            {
                break;
            }
        }
    }

    while (flag == 1)
    {
        getmaxyx(stdscr, max_y, max_x);
        clear();

        for (int i = 0; i < max_y; i++)
        {
            mvprintw(i, 0, "%s", buffer[i]);
        }

        mvchgat(y, x, 1, A_REVERSE, 0, NULL);
        move(y, x);

        ch = getch();

        switch (ch)
        {
        case KEY_UP:
            if (y > 0)
            {
                y--;
                x = x < strlen(buffer[y]) ? x : strlen(buffer[y]);
            }
            break;
        case KEY_DOWN:
            if (y < max_y - 1)
            {
                y++;
                x = x < strlen(buffer[y]) ? x : strlen(buffer[y]);
            }
            break;
        case KEY_LEFT:
            if (x > 0)
                x--;
            break;
        case KEY_RIGHT:
            if (x < strlen(buffer[y]))
                x++;
            break;
        case 27:
            flag = 2;
            break;
        case KEY_DC:
            if (buffer[y][x] != '\0')
            {
                memmove(&buffer[y][x], &buffer[y][x + 1], strlen(buffer[y]) - x);
            }
            break;
        case 24: // Ctrl+X
            flag = 0;
            break;
        case KEY_IC: // Press Insert instead of Control S
            FILE *file1 = fopen(fname, "w");
            if (file1)
            {
                for (int i = 0; i < max_y && buffer[i][0] != '\0'; i++)
                {
                    fprintf(file1, "%s\n", buffer[i]);
                }
                fclose(file1);
            }
            break;
        default:
            if (ch >= 32 && ch <= 126)
            {
                if (strlen(buffer[y]) < max_x - 1)
                {
                    memmove(&buffer[y][x + 1], &buffer[y][x], strlen(buffer[y]) - x + 1);
                    buffer[y][x] = ch;
                    x++;
                }
                else
                {
                    if (y < max_y)
                    {
                        memmove(&buffer[y + 2], &buffer[y + 1], (max_y - y) * sizeof(char *));
                        memset(buffer[y + 1], 0, SIZE);
                        x = 0;
                        y++;
                    }
                }
            }
            break;
        }
    }
    endwin();
    if (flag == 0)
    {
        int line = 0, word = 0, c = 0;
        for (int i = 0; i < SIZE; i++)
        {
            int temp = 0;
            for (int j = 0; j < SIZE; j++)
            {
                if (ini[i][j] != buffer[i][j])
                {
                    temp++;
                }
            }
            if (temp != 0)
            {
                c = c + temp;
                line++;
                // word = word + count_words(ini[i], buffer[i]);
            }
        }
        printf("Line: %d\nChar: %d\n", line, c);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    vi_editor(argv[1]);
    // printf("Done\n");

    return 0;
}