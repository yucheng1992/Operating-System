#ifndef CS202_OSPSH_H
#define CS202_OSPSH_H

/*
 * NYU CS 202 - Spring 2015 - Lab 2
 *   (Derived from Eddie Kohler's UCLA shell lab.)
 * Header file for Lab 2 - Shell processing
 * This file contains the definitions required for executing commands
 * parsed by the code in cmdparse.c
 */

#include "cmdparse.h"

/* Execute the command list. */
int cmd_line_exec(command_t *);

#endif
