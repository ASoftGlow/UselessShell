#include <stdlib.h>

#include "cmd_defs.h"

int main(void)
{
	UselessShell* us;
	if (us = useless_shell_create(cmds, cmds_len))
	{
		useless_shell_start(us);
		useless_shell_destroy(us);
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}