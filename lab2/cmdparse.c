/*
 * NYU CS 202 - Spring 2015 - Lab 2
 *   (Derived from Eddie Kohler's UCLA shell lab.)
 * Skeleton code for commandline parsing for Lab 2 - Shell processing
 * This file contains the skeleton code for parsing input from the command
 * line.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include "cmdparse.h"
#include "cmdrun.h"

/*
 * parsestate_t
 *
 *   The parsestate_t object represents the current state of the command line
 *   parser.  'parse_init' initializes a parsestate_t object for a command
 *   line, then calls to 'parse_gettoken' step through the command line one
 *   token at a time.  'parse_ungettoken' backs up one token.
 */


/*
 * parse_init(parsestate, line)
 *
 *   Initialize a parsestate_t object for a given command line.
 */

void
parse_init(parsestate_t *parsestate, char *input_line)
{
	parsestate->position = input_line;
	parsestate->last_position = NULL;
}


/*
 * parse_gettoken(parsestate, token)
 *
 *   Fetches the next token from the input line.
 *   The token's type is stored in 'token->type', and the token itself is
 *   stored in 'token->buffer'.  The parsestate itself is moved past the
 *   current token, so that the next call to 'parse_gettoken' will return the
 *   next token.
 *   Tokens are delimited by space characters. Any leading space is skipped.
 *
 */

void
parse_gettoken(parsestate_t *parsestate, token_t *token)
{
	int i;
	char *str = parsestate->position;	// current string
	int quote_state;
	int any_quotes;

	while (isspace((unsigned char) *str))
		str++;

	// Report TOK_END at the end of the command string.
	if (*str == '\0') {
		// Save initial position so parse_ungettoken() will work
		parsestate->last_position = parsestate->position;
		token->buffer[0] = '\0';	// empty token
		token->type = TOK_END;
		return;
	}


	// Stores the next token into 'token', and terminates the token
	// with a null character.  Some care is required to handle
	// quotes properly. We store at most TOKENSIZE - 1 characters
	// into 'token' (plus a terminating null character); longer
	// tokens cause an error.

	quote_state = any_quotes = 0;
	i = 0;
	while (*str != '\0'
	       && (quote_state || !isspace((unsigned char) *str))) {
		if (*str == '\"') {
			quote_state ^= 1;
			any_quotes = 1;
		} else {
			if (*str == '`') {
				quote_state ^= 2;
				any_quotes = 1;
			}
			if (i >= TOKENSIZE - 1)
				// Token too long; this is an error
				goto error;
			token->buffer[i++] = *str;
			if ((*str == '(' || *str == ')' || *str == ';')
			    && quote_state == 0) {
				if (i > 1)
					--i;
				else
					str++;
				break;
			}
		}
		str++;
	}
	if (quote_state)
		// Ended inside quotes; this is an error
		goto error;


	token->buffer[i] = '\0';	// end the token string


	// Save initial position so parse_ungettoken() will work
	parsestate->last_position = parsestate->position;
	// Move current position in place for the next token
	parsestate->position = str;


	// Examine the token and store its type in token->type.
	// Quoted special tokens, such as '">"', have type TOK_NORMAL.

	if (any_quotes)
		token->type = TOK_NORMAL;
	else if (strcmp(token->buffer, "<") == 0)
		token->type = TOK_LESS_THAN;
	else if (strcmp(token->buffer, ">") == 0)
		token->type = TOK_GREATER_THAN;
	else if (strcmp(token->buffer, "2>") == 0)
		token->type = TOK_2_GREATER_THAN;
	else if (strcmp(token->buffer, "&") == 0)
		token->type = TOK_AMPERSAND;
	else if (strcmp(token->buffer, ";") == 0)
		token->type = TOK_SEMICOLON;
	else if (strcmp(token->buffer, "|") == 0)
		token->type = TOK_PIPE;
	else if (strcmp(token->buffer, "&&") == 0)
		token->type = TOK_DOUBLEAMP;
	else if (strcmp(token->buffer, "||") == 0)
		token->type = TOK_DOUBLEPIPE;
	else if (strcmp(token->buffer, "(") == 0)
		token->type = TOK_OPEN_PAREN;
	else if (strcmp(token->buffer, ")") == 0)
		token->type = TOK_CLOSE_PAREN;
	else
		token->type = TOK_NORMAL;
	return;

 error:
	token->buffer[0] = '\0';
	token->type = TOK_ERROR;
}


/*
 * parse_ungettoken(parsestate)
 *
 *   Backs up the parsestate by one token.
 *   It's impossible to back up more than one token; if you call
 *   parse_ungettoken() twice in a row, the second call will fail.
 */

void
parse_ungettoken(parsestate_t *parsestate)
{
	// Can't back up more than one token.
	assert(parsestate->last_position != NULL);
	parsestate->position = parsestate->last_position;
	parsestate->last_position = NULL;
}


/*
 * cmd_alloc()
 *
 *   Allocates and returns a new blank command.
 */

command_t *
cmd_alloc(void)
{
	// Allocate memory for the command
	command_t *cmd = (command_t *) malloc(sizeof(*cmd));
	if (!cmd)
		return NULL;

	// Set all its fields to 0
	memset(cmd, 0, sizeof(*cmd));

	return cmd;
}


/*
 * cmd_free()
 *
 *   Frees all memory (the entire list) associated with a command.
 *
 *   EXERCISE:
 *        Fill in this function.
 *        Also free other structures pointed to by 'cmd', including
 *          'cmd->subshell' and 'cmd->next'.
 *        If you're not sure what to free, look at the other code in this file
 *          to see when memory for command_t data structures is allocated.
 */

void
cmd_free(command_t *cmd)
{
	int i;

	// It's OK to cmd_free(NULL).
	if (!cmd)
		return;
    if (cmd->subshell)
    {
            cmd_free(cmd->subshell);
            free(cmd->subshell);
    }
        
    if (cmd->next)
    {
        cmd_free(cmd->next);
        free(cmd->next);
    }
	/* Your code here. */
}


/*
 * cmd_parse(parsestate)
 *
 *   Parses a single command_t structure from the input string.
 *   Returns a pointer to the allocated command, or NULL on error
 *   or if the command is empty. (One example is if the end of the
 *   line is reached, but there are other examples too.)
 *
 *   EXERCISE:
 *        The current version of the function handles redirection and
 *        other cases but does not handle parentheses; your job is to
 *        make it do so.
 */

command_t *
cmd_parse(parsestate_t *parsestate)
{
    int i = 0;
    command_t *cmd = cmd_alloc();
    if (!cmd)
        return NULL;
    
    tokentype_t last_token_type = TOK_END;
    while (1) {
        // EXERCISE: Read the next token from 'parsestate'.
        
        // Normal tokens go in the cmd->argv[] array.
        // Redirection file names go into cmd->redirect_filename[].
        // Open parenthesis tokens indicate a subshell command.
        // Other tokens complete the current command
        // and are not actually part of it;
        // use parse_ungettoken() to save those tokens for later.
        
        // There are a couple errors you should check.
        // First, be careful about overflow on normal tokens.
        // Each command_t only has space for MAXTOKENS tokens in
        // 'argv'. If there are too many tokens, reject the whole
        // command.
        // Second, redirection tokens (<, >, 2>) must be followed by
        // TOK_NORMAL tokens containing file names.
        // Third, a parenthesized subcommand can't be part of the
        // same command as other normal tokens.  For example,
        // "echo ( echo foo )" and "( echo foo ) echo" are both errors.
        // (You should figure out exactly how to check for this kind
        // of error. Try interacting with the actual 'bash' shell
        // for some ideas.)
        // 'goto error' when you encounter one of these errors,
        // which frees the current command and returns NULL.
        
        // Hint: An open parenthesis should recursively call
        // command_line_parse(). The command_t structure has a slot
        // you can use for parens; figure out how to use it!
        if (i == MAXTOKENS)
            goto error;
        
        token_t token;
        parse_gettoken(parsestate, &token);
        
        switch (token.type) {
            case TOK_NORMAL:
                cmd->argv[i] = strdup(token.buffer);
                i++;
                break;
            case TOK_LESS_THAN:
                parse_gettoken(parsestate, &token);
                if (token.type == TOK_NORMAL)
                    cmd->redirect_filename[STDIN_FILENO] = strdup(token.buffer);
                else
                    goto error;
                break;
            case TOK_GREATER_THAN:
                parse_gettoken(parsestate, &token);
                if (token.type == TOK_NORMAL)
                    cmd->redirect_filename[STDOUT_FILENO] = strdup(token.buffer);
                else
                    goto error;
                break;
            case TOK_2_GREATER_THAN:
                parse_gettoken(parsestate, &token);
                if (token.type == TOK_NORMAL)
                    cmd->redirect_filename[STDERR_FILENO] = strdup(token.buffer);
                else
                    goto error;
                break;
            case TOK_OPEN_PAREN:
                if (last_token_type != TOK_NORMAL)
                    cmd->subshell = cmd_line_parse(parsestate, 1);
                else
                    goto error;
                break;
            case TOK_CLOSE_PAREN:
                parse_ungettoken(parsestate);
                goto done;
            case TOK_ERROR:
                goto error;
            case TOK_END:
                goto done;
            default:
                parse_ungettoken(parsestate);
                goto done;
        }
        last_token_type = token.type;
    }
    
done:
    // NULL-terminate the argv list
    cmd->argv[i] = 0;
    
    // EXERCISE: Make sure you return the right return value!
    
    if (i == 0 && cmd->subshell == NULL) {
        /* Empty command */
        cmd_free(cmd);
        return NULL;
    } else
        return cmd;
    
error:
    cmd_free(cmd);
    return NULL;
}


/*
 * cmd_line_parse(parsestate, in_parens)
 *
 *   Parses a command line from 'input' into a linked list of command_t
 *   structures. The head of the linked list is returned, or NULL is
 *   returned on error.
 *   If 'in_parens != 0', then cmd_line_parse() is being called recursively
 *   from cmd_parse().  A right parenthesis should end the "command line".
 *   But at the top-level command line, when 'in_parens == 0', a right
 *   parenthesis is an error.
 */
command_t *
cmd_line_parse(parsestate_t *parsestate, int in_parens)
{
	command_t *prev_cmd = NULL;
	command_t *head = NULL;
	command_t *cmd;
	token_t token;
	int r;

	// This loop has to deal with command syntax in a smart way.
	// Here's a nonexhaustive list of the behavior it should implement
	// when 'in_parens == 0'.

	// COMMAND                             => OK
	// COMMAND ;                           => OK
	// COMMAND && COMMAND                  => OK
	// COMMAND &&                          => error (can't end with &&)
	// COMMAND )                           => error (but OK if "in_parens")

	while (1) {
		// Parse the next command.
		cmd = cmd_parse(parsestate);
		if (!cmd)		// Empty commands are errors.
			goto error;

		if (prev_cmd)
			prev_cmd->next = cmd;
		else
			head = cmd;
		prev_cmd = cmd;

		// Fetch the next token to see how to connect this
		// command with the next command.  React to errors with
		// 'goto error'.  The ";" and "&" tokens require special
		// handling, since unlike other special tokens, they can end
		// the command line.

		parse_gettoken(parsestate, &token);
		switch (token.type) {
		case TOK_DOUBLEAMP:
		case TOK_DOUBLEPIPE:
		case TOK_PIPE:
			cmd->controlop = token.type;
			break;

		case TOK_SEMICOLON:
		case TOK_AMPERSAND:
			cmd->controlop = token.type;
			parse_gettoken(parsestate, &token);
			if (token.type == TOK_END || token.type == TOK_CLOSE_PAREN)
				goto ender;
			parse_ungettoken(parsestate);
			break;

		ender:
		case TOK_END:
		case TOK_CLOSE_PAREN:
			if ((token.type == TOK_END) == (in_parens != 0))
				goto error;
			goto done;

		default:
			goto error;
		}
	}

 done:
	// Check that the command line ends properly.
	if (prev_cmd
	    && prev_cmd->controlop != CMD_END
	    && prev_cmd->controlop != CMD_SEMICOLON
	    && prev_cmd->controlop != CMD_BACKGROUND)
		goto error;

	return head;

 error:
	cmd_free(head);
	return NULL;
}


/*
 * cmd_print(command, indent)
 *
 *   Prints a representation of the command to standard output.
 */

void
cmd_print(command_t *cmd, int indent)
{
	int argc, i;

	if (cmd == NULL) {
		printf("%*s[NULL]\n", indent, "");
		return;
	}

	for (argc = 0; argc < MAXTOKENS && cmd->argv[argc]; argc++)
		/* do nothing */;

	// More than MAXTOKENS is an error
	assert(argc <= MAXTOKENS);

	printf("%*s[%d args", indent, "", argc);
	for (i = 0; i < argc; i++)
		printf(" \"%s\"", cmd->argv[i]);

	// Print redirections
	if (cmd->redirect_filename[STDIN_FILENO])
		printf(" <%s", cmd->redirect_filename[STDIN_FILENO]);
	if (cmd->redirect_filename[STDOUT_FILENO])
		printf(" >%s", cmd->redirect_filename[STDOUT_FILENO]);
	if (cmd->redirect_filename[STDERR_FILENO])
		printf(" 2>%s", cmd->redirect_filename[STDERR_FILENO]);

	// Print the subshell command, if any
	if (cmd->subshell) {
		printf("\n");
		cmd_print(cmd->subshell, indent + 2);
	}

	printf("] ");
	switch (cmd->controlop) {
	case TOK_SEMICOLON:
		printf(";");
		break;
	case TOK_AMPERSAND:
		printf("&");
		break;
	case TOK_PIPE:
		printf("|");
		break;
	case TOK_DOUBLEAMP:
		printf("&&");
		break;
	case TOK_DOUBLEPIPE:
		printf("||");
		break;
	case TOK_END:
		// we write "END" as a dot
		printf(".");
		break;
	default:
		assert(0);
	}

	// Done!
	printf("\n");

	// if next is NULL, then controlop should be CMD_END, CMD_BACKGROUND,
	// or CMD_SEMICOLON
	assert(cmd->next || cmd->controlop == CMD_END
	       || cmd->controlop == CMD_BACKGROUND
	       || cmd->controlop == CMD_SEMICOLON);

	if (cmd->next)
		cmd_print(cmd->next, indent);
}
