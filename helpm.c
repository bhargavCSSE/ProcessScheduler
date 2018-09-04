#include <stdio.h>
#include <string.h>
#include "helpm.h"

//Help menu
int cmd_helpmenu(int n, char **a)
{
	(void)n;
	(void)a;
	printf("\t[run] <job> <time> <priority>\n");
	printf("\t[help] Print help menu\n");
	printf("\t[fcfs] FCFS policy\n\t[sjf] SJF policy\n\t[priority] Priority policy\n\t[quit] Exit AUbatch\n");
	return 0;
}
