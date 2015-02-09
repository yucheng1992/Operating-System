#ifndef CS202_CMDLINE_H
#define CS202_CMDLINE_H
#include <unistd.h>

/*
 * NYU CS 202 - Spring 2015 - Lab 2
 *   (Derived from Eddie Kohler's UCLA shell lab.)
 * Header file for commandline parsing for Lab 2 - Shell processing
 * This file contains the definitions required for parsing input
 * from the command line.
 */

/*
 * A command line "token" is an array of up to TOKENSIZE - 1 characters,
 * terminated by a null character ('\0') in the C tradition.
 */

#define TOKENSIZE 1024


/*
 * Shell tokens types have their own type.
 */

typedef enum {
	TOK_ERROR = -1,      // parse error, e.g. mismatched quotes
	TOK_END = 0,         // no token
	TOK_NORMAL,          // any non-special token
	TOK_LESS_THAN,       // '<'
	TOK_GREATER_THAN,    // '>'
	TOK_2_GREATER_THAN,  // '2>'
	TOK_SEMICOLON,       // ';'
	TOK_AMPERSAND,       // '&'
	TOK_PIPE,            // '|'
	TOK_DOUBLEAMP,       // '&&'
	TOK_DOUBLEPIPE,      // '||'
	TOK_OPEN_PAREN,      // '('
	TOK_CLOSE_PAREN      // ')'
} tokentype_t;


/*
 * The saved state of the token parser.
 * It is stored in this structure so that it can be saved across multiple
 * calls to next_token(), cmd_parse(), and cmd_line_parse().
 */

typedef struct {
	char *position;		// Portion of command line yet to be parsed
	char *last_position;	// 'position' from last token, allowing
				// one token's worth of rollback
} parsestate_t;


/*
 * A single token.
 * The token's string value is stored in 'buffer'.  Its type is 'type'.
 */

typedef struct {
	tokentype_t type;	    // Type of this token
	char buffer[TOKENSIZE];	    // Stores the current token
} token_t;


/*
 * parse_init(parsestate, line)
 *
 *   Initialize a parsestate_t object for a given command line.
 */

void parse_init(parsestate_t *parsestate, char *line);


/*
 * parse_gettoken(parsestate, token)
 *
 *   Parses a single token from the input line, stores its type into the
 *   token's 'type' field, and stores the token itself into its 'buffer' field.
 *   Also shifts the parsestate to point to the next token.
 *   Tokens are delimited by space characters. Any leading space is skipped.
 */

void parse_gettoken(parsestate_t *parsestate, token_t *token);


/*
 * parse_ungettoken(parsestate)
 *
 *   Backs up the parsestate by one token.
 *   For example, if the command line contains "a b c",
 *   then these calls should return the following tokens:
 *       parse_gettoken(parsestate, &token)  =>  TOK_NORMAL, "a"
 *       parse_gettoken(parsestate, &token)  =>  TOK_NORMAL, "b"
 *       parse_ungettoken(parsestate)
 *       parse_gettoken(parsestate, &token)  =>  TOK_NORMAL, "b"
 *       parse_gettoken(parsestate, &token)  =>  TOK_NORMAL, "c"
 */

void parse_ungettoken(parsestate_t *parsestate);


/*
 * Command joining operations have their own type.
 * Each controlop_t has the same integer value as the corresponding
 * tokentype_t. Note that not all token types are control operators!
 */

typedef enum {
	CMD_END = TOK_END,                  // end of command line
	CMD_SEMICOLON = TOK_SEMICOLON,      // ';'
	CMD_BACKGROUND = TOK_AMPERSAND,     // '&'
	CMD_PIPE = TOK_PIPE,                // '|'
	CMD_AND = TOK_DOUBLEAMP,            // '&&'
	CMD_OR = TOK_DOUBLEPIPE             // '||'
} controlop_t;


/*
 * A command_t represents a single shell command.
 *
 * Consists of a command to be executed, its arguments, and any redirections
 * to be applied to its stdin, stdout, and/or stderr.  Each command can
 * contain at most MAXTOKENS arguments. A command_t also stores a pointer to
 * the next command on the list, and the control operator that separates the
 * two commands. (Note that some control operators can be used at the end of a
 * command line, even when there is no next command.)
 *
 * For example, "ls", "ls -l", and "ls -l > out" can each be
 * represented using one command_t.
 */

#define MAXTOKENS 512

typedef struct command command_t;

struct command {
	char *argv[MAXTOKENS+1];    // command argument array
	                            // (terminated by NULL pointer)
	                            // argv[0] is the actual command.

	char *redirect_filename[3]; // filenames to be used for redirections,
	                            // indexed by file descriptor:
	                            // redirect_filename[0] is for stdin,
	                            // redirect_filename[1] is for stdout,
	                            // redirect_filename[2] is for stderr

	command_t *subshell;        // pointer to subshell command line
	                            // (only used for parentheses)

	controlop_t controlop;      // control operator applied between
	                            // this command and the next

	command_t *next;            // pointer to the next command
	                            // in the command line
};


/* Allocates and returns a new blank command. */
command_t *cmd_alloc(void);

/* Frees a command. */
void cmd_free(command_t *command);

/* Parses a single command from a command line parameter, using next_token. */
command_t *cmd_parse(parsestate_t *parsestate);

/* Parses a linked list of commands using cmd_parse. */
command_t *cmd_line_parse(parsestate_t *parsestate, int in_parens);

/* Prints a command for debugging. */
void cmd_print(command_t *cmdlist, int indent);

#endif
