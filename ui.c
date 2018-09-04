#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "ui.h"

//Commandline UI

void *UI( void *ptr)
{
	char *buffer;
        size_t bufsize = 64;
        
        buffer = (char*) malloc(bufsize * sizeof(char));
        if (buffer == NULL) {
 		perror("malloc buffer failed");
 		exit(1);
	}
	
	printf("Welcome to AUbatch scheduler\n");
	printf("Please type h for help menu\n");
	printf("*******************************************************************\n");
	printf("HOW TO USE?\n");
	printf("Type \"run init <time> <\"1\">\"\n");
	printf("It allows user to choose how much time they need to give input arguments\n");
	printf("Choose policy and type it i.e. sjf or pri\n");
	printf("Submit jobs with \"run <Jobname><Burst_time><priority>\"\n");
	printf("Wait while the time you chose to add jobs finish\n");
	printf("Thank you, Happy scheduling>\n");
	printf("*******************************************************************\n");
      		while (1) {
		printf("> ");
		getline(&buffer, &bufsize, stdin);
		cmd_dispatch(buffer);
	}
}
