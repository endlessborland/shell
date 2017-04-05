/*
  main.c
  shell

  Created by Денис Скоробогатов on 14.02.17.
  Copyright © 2017 Denis Skorobogatov. All rights reserved.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


#define MAXINPUT 256
#define DIRLENGTH 1024

extern int errno;
extern char **environ;

char *readline()
{
    char *input = calloc(1, sizeof(char)); /* init the array */
    char c; int i = 0; char a = '%';
    while ((c = getchar()) != '\n')
    {
        if (c == EOF && i == 0) /* it happens, for some reason */
            continue;
        if (i == 0 && c == ' ') /* deleting first spaces */
            continue;
        if (a == c && c == ' ') /* deleting repeating spaces */
            continue;
        if (i < MAXINPUT)
        {
            a = c;
            input = realloc(input, (i+1) * sizeof(char));
            input[i] = c;
            i++;
        }
    }
    if (input[i - 1] == ' ') /* deleting the last space */
    {
        input[i - 1] = '\0';
        return input;
    }
    input = realloc(input, (i + 1) * sizeof(char)); /* finishing a string */
    input[i] = '\0';
    return input;
}

int wrdlen(char *string) /* counts symbols before space or \0 */
{
    int length = 0;
    int i = 0;
    while (!(*(string + sizeof(char) * i) == ' ' || *(string + sizeof(char) * i) == '\0'))
    {
        length++;
        i++;
    }
    return length;
}

int parse(char *input, char ***arguments)
{
    char **_arguments;
    int args = 0;
    int length, lastindex = 0;
    int i = 0, j;
    while (input[i] != '\0') /* counting arguments */
    {
        if (input[i] == ' ')
            args++;
        i++;
    }
    _arguments = calloc((args + 2), sizeof(char*)); /* allocating memory for args */
    for (j = 0; j <= args; j++)
    {
        length = wrdlen(&input[lastindex]);
        _arguments[j] = calloc(length + 1, sizeof(char));
        for (i = 0; i < length; i++)
            _arguments[j][i] = input[lastindex + i];
        _arguments[j][i] = '\0';
        lastindex += length + 1;
    }
    _arguments[j] = NULL;
    *arguments = _arguments;
    return args;
}

void startprocess(char **arguments)
{
    pid_t pid;
    int status;
    pid = vfork();
    if (pid == 0)
    {
        if (execvp(*arguments, arguments) < 0)
            printf("*** ERROR, execvp failed\n");
        exit(errno);
    }
    if (pid < 0)
    {
        printf("Fork failed\n");
        return;
    }
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && status)
        printf("*** Exit value = %d\n",WEXITSTATUS(status));
}

int main(int argc, const char * argv[]) {
    char **arguments;
    int args, i;
    char *cwd;
    char *user = getenv("USER");
    char symbol = '$';
    if (!strncmp(user, "root", 4) && strlen(user) == 4)
        symbol = '#';
    while(1) {
        cwd = getcwd(NULL, DIRLENGTH);
        printf("SHELL@%s:%s%c ", user, cwd, symbol);
        args = parse(readline(), &arguments);
        if (!(strncmp(*arguments, "cd", 2)) && strlen(*arguments) == 2)
        {
            chdir(arguments[1]);
            continue;
        }
        if (!strncmp(*arguments, "exit", 4) && strlen(*arguments) == 4)
            break;
        startprocess(arguments);
        for (i = 0; i < args; i++)
        {
            free(arguments[i]);
        }
        free(arguments);
        free(cwd);
    }
    return 0;
}
