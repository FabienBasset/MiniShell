#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"

#ifndef _SHELL_COMMANDS
#define _SHELL_COMMANDS
#endif // _SHELL_COMMANDS

void cd_command(char ** args);
void export_command(char ** args,SHELL_Context * context);
void unset_command(char ** args,SHELL_Context * context);
void exit_command(SHELL_Context * context);


