/*
 * NYU CS 202 - Spring 2015 - Lab 2
 *   (Derived from Eddie Kohler's UCLA shell lab.)
 * Skeleton code for Lab 2 - Shell processing
 * This file contains skeleton code for executing parsed commands.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "cmdparse.h"
#include "cmdrun.h"

/* cmd_exec(cmd, pass_pipefd)
 *
 *   Execute the single command specified in the 'cmd' command structure.
 *
 *   The 'pass_pipefd' argument is used for pipes.
 *
 *     On input, '*pass_pipefd' is the file descriptor that the
 *     current command should use to read the output of the previous
 *     command. That is, it's the "read end" of the previous
 *     pipe, if there was a previous pipe; if there was not, then
 *     *pass_pipefd will equal STDIN_FILENO.
 *
 *     On output, cmd_exec should set '*pass_pipefd' to the file descriptor
 *     used for reading from THIS command's pipe (so that the next command
 *     can use it). If this command didn't have a pipe -- that is,
 *     if cmd->controlop != PIPE -- then this function should set
 *     '*pass_pipefd = STDIN_FILENO'.
 *
 *   Returns the process ID of the forked child, or < 0 if some system call
 *   fails.
 *
 *   Besides handling normal commands, redirection, and pipes, you must also
 *   handle three internal commands: "cd", "exit", and "our_pwd".
 *   (Why must "cd", "exit", and "our_pwd" (a version of "pwd") be implemented
 *   by the shell, versus simply exec()ing to handle them?)
 *
 *   Note that these special commands still have a status!
 *   For example, "cd DIR" should return status 0 if we successfully change
 *   to the DIR directory, and status 1 otherwise.
 *   Thus, "cd /tmp && echo /tmp exists" should print "/tmp exists" to stdout
 *      if and only if the /tmp directory exists.
 *   Not only this, but redirections should work too!
 *   For example, "cd /tmp > foo" should create an empty file named 'foo';
 *   and "cd /tmp 2> foo" should print any error messages to 'foo'.
 *
 *   Some specifications:
 *
 *       --subshells:
 *         the exit status should be either 0 (if the last
 *         command would have returned 0) or 5 (if the last command
 *         would have returned something non-zero). This is not the
 *         behavior of bash.
 *
 *       --cd:
 *
 *          this builtin takes exactly one argument besides itself (this
 *          is also not bash's behavior). if it is given fewer
 *          ("cd") or more ("cd foo bar"), that is a syntax error.  Any
 *          error (syntax or system call) should result in a non-zero
 *          exit status. Here is the specification for output:
 *
 *                ----If there is a syntax error, your shell should
 *                display the following message verbatim:
 *                   "cd: Syntax error! Wrong number of arguments!"
 *
 *                ----If there is a system call error, your shell should
 *                invoke perror("cd")
 *
 *       --our_pwd:
 *
 *          This stands for "our pwd", which prints the working
 *          directory to stdout, and has exit status 0 if successful and
 *          non-zero otherwise. this builtin takes no arguments besides
 *          itself. Handle errors in analogy with cd. Here, the syntax
 *          error message should be:
 *
 *              "pwd: Syntax error! Wrong number of arguments!"
 *
 *       --exit:
 *
 *          As noted in the lab, exit can take 0 or 1 arguments. If it
 *          is given zero arguments (besides itself), then the shell
 *          exits with command status 0. If it is given one numerical
 *          argument, the shell exits with that numerical argument. If it
 *          is given one non-numerical argument, do something sensible.
 *          If it is given more than one argument, print an error message,
 *          and do not exit.
 *
 *
 *   Implementation hints are given in the function body.
 */

typedef enum
{
    ERROR_PIPEFAIL = -1,
    ERROR_NOCHILD = -2,
    ERROR_DUP = -3,
    ERROR_OPEN = -4,
    ERROR_NULL_ARG = -5
} error_code_t;

static pid_t
cmd_exec(command_t *cmd, int *pass_pipefd)
{
    //(void)pass_pipefd;      // get rid of unused warning
	//pid_t pid = -1;		// process ID for child
	//int pipefd[2];		// file descriptors for this process's pipe
    pid_t pid = -1;         // process ID for child
    int pipefd[2] = {-1, -1};       // file descriptors for this process's pipe
    int rd_fd[3] = { -1, -1, -1};   // file descriptors for the redirection filenames
    
    int argc;
    
    if (!pass_pipefd || !cmd)
    {
        return ERROR_NULL_ARG;
    }
    
    // count args
    for (argc = 0; argc < MAXTOKENS && cmd->argv[argc]; argc++)
    {
        ;
    }
    
    // Create a pipe, if this command is the left-hand side of a pipe.
    // Return ERROR_PIPEFAIL if the pipe fails.
    if (cmd->controlop == CMD_PIPE)
    {
        if (pipe(pipefd) < 0 || !cmd->next)
        {
            return ERROR_PIPEFAIL;
        }
    }
    
    // Fork the child and execute the command in that child.
    // You will handle all redirections by manipulating file descriptors.
    pid = fork();
    
    if (pid < 0)
    {
        return ERROR_NOCHILD;
    }
    
    // This section is fairly long.  It is probably best to implement this
    // part in stages, checking it after each step.  For instance, first
    // implement just the fork and the execute in the child.  This should
    // allow you to execute simple commands like 'ls'.  Then add support
    // for redirections: commands like 'ls > foo' and 'cat < foo'.  Then
    // add parentheses, then pipes, and finally the internal commands
    // 'cd' and 'exit'.
    
    if (pid == 0)   // i'm in the child process
    {
#ifdef DEBUG_CHILD
        sleep(30);  // used for debugging child process
#endif
        int ret = -1;  // used when returning from child
        
        // In the child, you should:
        
        //    1. Set up stdout to point to this command's pipe, if necessary.
        if (cmd->controlop == CMD_PIPE)
        {
            if (dup2(pipefd[1], STDOUT_FILENO) < 0)
            {
                ret = ERROR_DUP;
                goto error;
            }
        }
        
        //    2. Set up stdin to point to the PREVIOUS command's pipe (that
        //       is, *pass_pipefd), if appropriate.
        if (*pass_pipefd >= 0)
        {
            if (dup2(*pass_pipefd, STDIN_FILENO) < 0)
            {
                ret = ERROR_DUP;
                goto error;
            }
            
            *pass_pipefd = -1;
        }
        
        //    3. Close some file descriptors.  Hint: Consider the read end
        //       of this process's pipe.
        if (!cmd->subshell && pipefd[0] >= 0)
        {
            close(pipefd[0]);
        }
        
        //    4. Set up redirections.
        //       Hint: For output redirections (stdout and stderr), the 'mode'
        //       argument of open() should be set to 0666.
        if (cmd->redirect_filename[0] && strcmp(cmd->redirect_filename[0],"\0") != 0)
        {
            rd_fd[0] = open(cmd->redirect_filename[0], O_RDONLY);
            if (rd_fd[0] < 0)
            {
                ret = ERROR_OPEN;
                goto error;
            }
            
            if (dup2(rd_fd[0], STDIN_FILENO) < 0)
            {
                ret = ERROR_DUP;
                goto error;
            }
        }
        if (cmd->redirect_filename[1] && strcmp(cmd->redirect_filename[1],"\0") != 0)
        {
            rd_fd[1] = open(cmd->redirect_filename[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (rd_fd[1] < 0)
            {
                ret = ERROR_OPEN;
                goto error;
            }
            
            if (dup2(rd_fd[1], STDOUT_FILENO) < 0)
            {
                ret = ERROR_DUP;
                goto error;
            }
        }
        if (cmd->redirect_filename[2] && strcmp(cmd->redirect_filename[2],"\0") != 0)
        {
            rd_fd[2] = open(cmd->redirect_filename[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (rd_fd[2] < 0)
            {
                ret = ERROR_OPEN;
                goto error;
            }
            
            if (dup2(rd_fd[2], STDERR_FILENO) < 0)
            {
                ret = ERROR_DUP;
                goto error;
            }
        }
        
        //    5. Execute the command.
        //       There are some special cases:
        //       a. Parentheses.  Execute cmd->subshell.  (How?)
        //       b. A null command (no subshell, no arguments).
        //          Exit with status 0.
        //       c. "exit".
        //       d. "cd".
        if (cmd->subshell)
        {
            if (cmd->controlop == CMD_PIPE)
            {
                dup2(pipefd[0],STDIN_FILENO);
            }
            
            exit(cmd_line_exec(cmd->subshell));
        }
        // if no args or command is exit, q, makeq, waitq
        else if (argc == 0 || 0 == strcmp(cmd->argv[0],"exit") || 0 == strcmp(cmd->argv[0],"q")
                 || 0 == strcmp(cmd->argv[0],"makeq") || 0 == strcmp(cmd->argv[0],"waitq"))
        {
            exit(0);
        }
        else if (0 == strcmp(cmd->argv[0],"cd"))
        {
            int ret = chdir((const char*)cmd->argv[1]);
            
            if (ret == -1)
            {
                perror("cd");
            }
            
            exit(ret);
        }
        else
        {
            exit(execvp(cmd->argv[0], cmd->argv));
        }
        
    error:
        close(rd_fd[0]);
        close(rd_fd[1]);
        close(rd_fd[2]);
        
        return ret;
    }
    else    // i'm in the parent process
    {
        if (cmd->redirect_filename[2] && strcmp(cmd->redirect_filename[2],"\0") != 0)
        {
            rd_fd[2] = open(cmd->redirect_filename[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (rd_fd[2] < 0)
            {
                return ERROR_OPEN;
            }
            
            if (dup2(rd_fd[2], STDERR_FILENO) < 0)
            {
                return ERROR_DUP;
            }
        }
        
        // In the parent, you should:
        //    1. Close some file descriptors.  Hint: Consider the write end
        //       of this command's pipe, and one other fd as well.
        if (pipefd[1] >= 0)
        {
            close(pipefd[1]);
        }
        
        //    2. Handle the special "exit" and "cd" commands.
        //
        // "cd" error note:
        //      - Upon syntax errors: Display the message
        //        "cd: Syntax error on bad number of arguments"
        //      - Upon system call errors: Call perror("cd")
        //
        // "cd" Hints:
        //    For the "cd" command, you should change directories AFTER
        //    the fork(), not before it.  Why?
        //    Design some tests with 'bash' that will tell you the answer.
        //    For example, try "cd /tmp ; cd $HOME > foo".  In which directory
        //    does foo appear, /tmp or $HOME?  If you chdir() BEFORE the fork,
        //    in which directory would foo appear, /tmp or $HOME?
        //
        //    EXTRA CREDIT: Our "cd" solution changes the
        //    directory both in the parent process and in the child process.
        //    This introduces a potential race condition.
        //    Explain what that race condition is, and fix it.
        //    Hint: Investigate fchdir().
        
        if (cmd->argv[0])
        {
            if (0 == strcmp(cmd->argv[0],"cd"))
            {
                int ret = chdir((const char*)cmd->argv[1]);
                if (ret == -1)
                {
                    perror("cd");
                }
            }
            else if (0 == strcmp(cmd->argv[0],"exit"))
            {
                exit(0);
            }
            else if (0 == strcmp(cmd->argv[0],"makeq"))
            {
                if (!cmd->argv[1] || !cmd->argv[2] || cmd->argv[3])
                {
                    fprintf(stderr, "makeq: Syntax error\n");
                }
                
                    //command_queue_makeq(cmd->argv[1], cmd->argv[2]);
            }
            else if (0 == strcmp(cmd->argv[0],"q"))
            {
                if (!cmd->argv[1] || !cmd->argv[2])
                {
                    fprintf(stderr, "q: Syntax error\n");
                }
                
                char* name = cmd->argv[1];
                
                // in order to use the same command for the queue we must remove
                // the command "q" (i.e. argv[0]) and the queue "name" (i.e. argv[1])
                // to do this simply set argv[i] = argv[i+2];
                unsigned int i = 0;
                for (i = 0; cmd->argv[i] != NULL && i < MAXTOKENS-1; i++)
                    cmd->argv[i] = cmd->argv[i+2];
                
                
                // set last two args to NULL (pointless if they are already NULL)
                cmd->argv[MAXTOKENS-1] = NULL;
                cmd->argv[MAXTOKENS] = NULL;
                
                //command_queue_q(name, cmd);
            }
            else if (0 == strcmp(cmd->argv[0],"waitq"))
            {
                if (!cmd->argv[1] || cmd->argv[2])
                {
                    fprintf(stderr, "waitq: Syntax error\n");
                }
                
                //command_queue_waitq(cmd->argv[1]);
            }
        }
        
        //    3. Set *pass_pipefd as appropriate.
        if (cmd->controlop == CMD_PIPE)
        {
            *pass_pipefd = pipefd[0];
        }
        else
        {
            *pass_pipefd = STDIN_FILENO;
        }
    }
    
    // return the child process ID
    return pid;
}


/* cmd_line_exec(cmdlist)
 *
 *   Execute the command list.
 *
 *   Execute each individual command with 'cmd_exec'.
 *   String commands together depending on the 'cmdlist->controlop' operators.
 *   Returns the exit status of the entire command list, which equals the
 *   exit status of the last completed command.
 *
 *   The operators have the following behavior:
 *
 *      CMD_END, CMD_SEMICOLON
 *                        Wait for command to exit.  Proceed to next command
 *                        regardless of status.
 *      CMD_AND           Wait for command to exit.  Proceed to next command
 *                        only if this command exited with status 0.  Otherwise
 *                        exit the whole command line.
 *      CMD_OR            Wait for command to exit.  Proceed to next command
 *                        only if this command exited with status != 0.
 *                        Otherwise exit the whole command line.
 *      CMD_BACKGROUND, CMD_PIPE
 *                        Do not wait for this command to exit.  Pretend it
 *                        had status 0, for the purpose of returning a value
 *                        from cmd_line_exec.
 */
int
cmd_line_exec(command_t *cmdlist)
{
    int cmd_status = 0;         // status of last command executed
    int pipefd = -1;  // read end of last pipe
    int zombie_status;
    
    while (cmdlist)
    {
        int wp_status; // Hint: use for waitpid's status argument!
        // Read the manual page for waitpid() to
        // see how to get the command's exit
        // status (cmd_status) from this value.
        
        int ret_if_exit;
        
        pid_t pid = cmd_exec(cmdlist, &pipefd);
        
        if (pid < 0)
        {
#ifdef PRINT_DEBUG_TEXT
            printf("error: %d\n", pid);
#endif
            abort();
        }
        
        switch (cmdlist->controlop)
        {
            case CMD_END:
            case CMD_SEMICOLON:
                waitpid(pid, &wp_status, 0);
                if (WIFEXITED(wp_status))
                {
                    cmd_status = WEXITSTATUS(wp_status);
                }
                else
                {
                    cmd_status = wp_status;
                    goto done;
                }
                break;
            case CMD_BACKGROUND:
            case CMD_PIPE:
                break;
            case CMD_AND:
                waitpid(pid, &wp_status, 0);
                ret_if_exit = WIFEXITED(wp_status);
                if (!ret_if_exit || (ret_if_exit && (WEXITSTATUS(wp_status) != 0)))
                {
                    cmd_status = wp_status;
                    goto done;
                }
                
                break;
            case CMD_OR:
                waitpid(pid, &wp_status, 0);
                ret_if_exit = WIFEXITED(wp_status);
                if (!ret_if_exit || (ret_if_exit && (WEXITSTATUS(wp_status) == 0)))
                {
                    cmd_status = wp_status;
                    goto done;
                }
                break;
            default:
                abort(); // error, not one of the control ops
        }
        
        cmdlist = cmdlist->next;
    }
    
done:
    // clean up zombies
    while (waitpid(-1, &zombie_status, WNOHANG) > 0);
    
    return cmd_status;
}
