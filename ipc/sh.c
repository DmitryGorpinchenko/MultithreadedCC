#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_ARGS 10

char *line();
int exe(const char *cmd_pipe, const char *out_path);

int main(void)
{
    char *cmd_pipe = line();
    
    if (cmd_pipe == NULL) {
        return 1;
    }
    
    int res = exe(cmd_pipe, "/home/box/result.out"/*"result.out"*/); // alternative output path for local testing
    
    free(cmd_pipe);
    return res;
}

static const char *cmd_delim = "|";
static const char *arg_delim = " ";

enum state {
    BEGIN  = 1 << 0, 
    FIRST  = 1 << 1, 
    MIDDLE = 1 << 2, 
    LAST   = 1 << 3, 
    END    = 1 << 4
};

struct cmd_pipe {
    char *cmds, *next;
    char *cmd_ctx, *arg_ctx;
    int flags;
};

struct cmd_pipe *init_pipe(const char *data);
void free_pipe(struct cmd_pipe *pipe);
int next(struct cmd_pipe *pipe, char **argv);
void lookahead(struct cmd_pipe *pipe);

int exe(const char *cmd_pipe, const char *out_path)
{
    char *argv[MAX_ARGS];
    
    struct cmd_pipe *p = init_pipe(cmd_pipe);
    
    if (p == NULL) {
        return 1;
    }
    
    int in = -1;
    while (next(p, argv)) {
        int out[2];
        if (pipe(out) < 0) {
            break;
        }
        
        int res;
        if ((res = fork()) == 0) { // child
            if (!(p->flags & FIRST)) {
                if (dup2(in, STDIN_FILENO) < 0) {
                    exit(1);
                }
            }
            if (!(p->flags & LAST)) {
                if (dup2(out[1], STDOUT_FILENO) < 0) {
                    exit(1);
                }
            } else {
                int fd;
                if (((fd = open(out_path, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0)
                ||  (dup2(fd, STDOUT_FILENO) < 0)) {
                    exit(1);
                }
                close(fd);
            }
            close(in);
            close(out[0]);
            close(out[1]);
            if (execvp(*argv, argv) < 0) {
                exit(1);
            }
        }
        
        close(in);
        in = out[0];
        close(out[1]); // close write end in order to recieve EOF on read end
        
        if (res < 0) {
            break; // fork() failed
        }
    }
    
    int res = (p->flags == END);
    
    int status;
    while (waitpid(-1, &status, 0) > 0) {
        res &= (WIFEXITED(status) && !WEXITSTATUS(status));
    }
    
    if (!res) {
        remove(out_path); // cleanup output file on pipeline failure
    }
    close(in);
    free_pipe(p);
    return !res; // zero on success
}

int next(struct cmd_pipe *pipe, char **argv)
{
    if (pipe->next) {
        char **it = argv;
        for (char *p = pipe->next; *it++ = strtok_r(p, arg_delim, &pipe->arg_ctx); p = NULL)
            ;
    }
    int res = pipe->next && *argv;
    lookahead(pipe);
    return res;
}

void lookahead(struct cmd_pipe *pipe)
{
    if (pipe->flags == END) {
        return;
    }

    pipe->next = strtok_r(NULL, cmd_delim, &pipe->cmd_ctx);
    
    if (pipe->flags == BEGIN) {
        pipe->flags = FIRST;
        if (pipe->next == NULL) {
            pipe->flags |= LAST;
        }
    } else if (pipe->flags & LAST) {
        pipe->flags = END;
    } else if (pipe->next == NULL) {
        pipe->flags = LAST;
    } else {
        pipe->flags = MIDDLE;
    }
}

struct cmd_pipe *init_pipe(const char *data)
{
    if (data == NULL) {
        return NULL;
    }

    struct cmd_pipe *pipe = malloc(sizeof(struct cmd_pipe));
    
    if (pipe) {
        pipe->flags = BEGIN;
        pipe->cmd_ctx = pipe->arg_ctx = NULL;
        if (((pipe->cmds = strdup(data)) == NULL)
        ||  ((pipe->next = strtok_r(pipe->cmds, cmd_delim, &pipe->cmd_ctx)) == NULL)) {
            free_pipe(pipe);
            return NULL;
        }
    }
    return pipe;
}

void free_pipe(struct cmd_pipe *pipe)
{
    if (pipe) {
        free(pipe->cmds);
        free(pipe);
    }
}

char *line()
{
    char *buf = NULL;
    size_t n = 0;
    int res = getline(&buf, &n, stdin);
    
    if (res > 0) {
        buf[res - 1] = '\0';
        return buf;
    }   
    free(buf);
    return NULL;
}

