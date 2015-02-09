/*
 * NYU CS 202 - Spring 2015 - Lab 2
 *   (Derived from Eddie Kohler's UCLA shell lab.)
 * Skeleton code for Lab 2 - Shell processing
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

/*
 * Main function for shell.
 */
int
main(int argc, char *argv[])
{
	int quiet = 0, parseonly = 0;
	char input[BUFSIZ];
	int i = 0, r = 0;

	// Check for '-q', '-p' option:
	//    -q: be quiet -- print no prompts
	//    -p: parse only -- do not run the cmd
	for (i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-q") == 0) {
			quiet = 1;
		} else if(strcmp(argv[i], "-p") == 0) {
			parseonly = 1;
		}
	}

	while (!feof(stdin)) {
		parsestate_t parsestate;
		command_t *cmdlist;
		// Print the prompt
		if (!quiet) {
			printf("cs202$ ");
			fflush(stdout);
		}

		// Read a string, checking for error or EOF
		if (fgets(input, BUFSIZ, stdin) == NULL) {
			if (ferror(stdin) && errno == EINTR) {
				cmd_line_exec(0);
				continue;
			}
			if (ferror(stdin))
				// This function prints a description of the
				// error, preceded by 'cs202_sp15: '.
				perror("cs202_sp15");
			break;
		}

		// build the command list
		parse_init(&parsestate, input);

		cmdlist = cmd_line_parse(&parsestate, 0);
		if (!cmdlist) {
			printf("Syntax error\n");
			continue;
		}

		// print the command list
		if (!quiet) {
			cmd_print(cmdlist, 0);
			// why do we need to do this?
			fflush(stdout);
		}

                // this actually invokes the command list.
                // you will fill in cmd_line_exec.
		if (!parseonly && cmdlist)
			cmd_line_exec(cmdlist);
                if (cmdlist)
                        cmd_free(cmdlist);

	}

	return 0;
}
