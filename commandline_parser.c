/**************************************************************************************************
					   Project - 3					
					AU batch scheduler
***************************************************************************************************/
/*
Created by - BHARGAV JOSHI
Course - Advanced Operating Systems
Submitted to - Dr. Qin
*/
/*
Note: Reference was taken from Dr. Qin's AUbatch_sample.c and commandline_parser.c.
Similar function names from Dr. Qin's codes but everything is modified to incorporate new logic.
*/

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "dispatcher.h"
#include "scheduler.h"
#include "ui.h"
#include "helpm.h"

/* Error Codes */
#define EINVAL       1
#define E2BIG        2

#define MAXMENUARGS  4 
#define MAXCMDLINE   64 

#define CMD_BUF_SIZE 10 /* The size of the command queueu */
#define NUM_OF_CMD   10  /*Number of scheduler->dispatcher operations*/
#define MAX_CMD_LEN  256 /* The longest commandline length */

pthread_mutex_t cmd_queue_lock;     /*	Lock for critical sections */
pthread_mutex_t cmd_queue_run_lock; /*	UI to scheduler mutex*/
pthread_cond_t cmd_buf_not_full;    /*	Condition variable for buf_not_full */
pthread_cond_t cmd_buf_not_empty;   /*	Condition variable for buf_not_empty */
pthread_cond_t job_submit;	    /*	Condition variable for job_submit */

//Structure of BUFFER
struct queue
{
char name[10];
int burst_time;
int priority;
};

/* Global shared variables */
unsigned int buf_head;
unsigned int buf_tail;
unsigned int count;
unsigned int lock1 = 0;
char cmd[30];
char *param_list[3];
struct queue *cmd_buffer[CMD_BUF_SIZE];

//UI functions definitions referenced from Dr. Qin's code
void menu_execute(char *line, int isargs); 
int cmd_run(int nargs, char **args); 
int cmd_quit(int nargs, char **args); 
void showmenu(const char *name, const char *x[]);
int cmd_helpmenu(int n, char **a);
int cmd_dispatch(char *cmd);
int cmd_fcfs(int n, char **a);
int cmd_sjf(int n, char **a);
int cmd_priority(int n, char **a);
void policy_check(void);

//Core functions definitions
void *UI( void *ptr );		/* To simulate user interface*/
void *commandline( void *ptr ); /* To simulate job submissions and scheduling */
void *executor( void *ptr );    /* To simulate job execution */
void *execv_call(struct queue *param);/*To run processes using execv()*/

void policy_check(void){
	//Function to apply proper sorting in job queue
	struct queue *key;
	int i = 0,j;
	switch(policy){
	case 0:
		/*Sorting Priority*/
		for(i=0;i<buf_head-1;i++){
			for(j=0;j<buf_head-i-1;j++){
				if(cmd_buffer[j]->priority > cmd_buffer[j+1]->priority)
				{
				key = cmd_buffer[j];
				cmd_buffer[j] = cmd_buffer[j+1];
				cmd_buffer[j+1] = key;
				} 
			}	
		}
		break;
	case 1:
		/*Sorting Burst Time*/
		for(i=0;i<buf_head-1;i++){
			for(j=0;j<buf_head-i-1;j++){
				if(cmd_buffer[j]->burst_time > cmd_buffer[j+1]->burst_time)
				{
				key = cmd_buffer[j];
				cmd_buffer[j] = cmd_buffer[j+1];
				cmd_buffer[j+1] = key;
				} 
			}	
		}
		break;

	case 2:
		/*This is for FCFS*/
		/*Scheduler and dispatcher are FCFS by default so no extra code needed*/
		break;

	default:
		break;	
	}
}

void *execv_call(struct queue *param){
	char *arg[5];
	pid_t pid;
	arg[0] = "process";
	arg[1] = param->name;
	sprintf(arg[2],"%d",param->burst_time);
	sprintf(arg[3],"%d",param->priority);
	arg[4] = NULL;
	/*Fork() to host an execv() process*/
	switch ((pid = fork()))
  	{
   	case -1:
   	 	perror("fork failed");
      		break;
    	case 0:
		if(execv("process",arg)==-1){ //Execution of execv()
			perror("Fail\n");
			exit(0);
		}
      		break;
    	default:
		pthread_mutex_unlock(&cmd_queue_lock);
		sleep(atoi(arg[2])); //Make this process last till process finishes
		pthread_mutex_lock(&cmd_queue_lock);
      		return 0;
  	}
}

//The run command - submit a job
int cmd_run(int nargs, char **args) {
	int i=1;
	int counter = nargs;
	pthread_mutex_lock(&cmd_queue_run_lock);
	if (nargs != 4) {
		printf("Usage: run <job> <time> <priority>\n");
	return EINVAL;
	}
	counter--;
	strcpy(cmd,"./process ");
	while(counter>0){
		param_list[i-1] = args[i];
		strcat(cmd,param_list[i-1]);
		strcat(cmd," ");
		counter--;
		i++;
	}
	lock++;
	pthread_cond_signal(&job_submit);
	pthread_mutex_unlock(&cmd_queue_run_lock);
}

//Quit command
int cmd_quit(int nargs, char **args) {
	printf("Thank you for using AUbatch!\n");
        exit(0);
}

//fcfs policy
int cmd_fcfs(int n, char **a)
{
	(void)n;
	(void)a;
	policy = 2;
	printf("Policy is changed to FCFS\n");
	return 0;
}

//sjf policy
int cmd_sjf(int n, char **a)
{
	(void)n;
	(void)a;
	policy = 1;
	printf("Policy is changed to SJF\n");
	return 0;
}

//priority policy
int cmd_priority(int n, char **a)
{
	(void)n;
	(void)a;
	policy = 0;
	printf("Policy is changed to Priority scheduling\n");
	return 0;
}

//Command table
static struct {
	const char *name;
	int (*func)(int nargs, char **args);
} cmdtable[] = {
	/* commands: single command must end with \n */
	{ "?\n",	cmd_helpmenu },
	{ "h\n",	cmd_helpmenu },
	{ "help\n",	cmd_helpmenu },
	{ "r",		cmd_run },
	{ "run",	cmd_run },
	{ "q\n",	cmd_quit },
	{ "quit\n",	cmd_quit },
        {"fcfs\n", 	cmd_fcfs},
	{"sjf\n", 	cmd_sjf},
	{"priority\n", 	cmd_priority},
	{"pri\n", 	cmd_priority},
};

//Execute a command
int cmd_dispatch(char *cmd)
{
	time_t beforesecs, aftersecs, secs;
	u_int32_t beforensecs, afternsecs, nsecs;
	char *args[MAXMENUARGS];
	int nargs=0;
	char *word;
	char *context;
 	int i, result;

	for (word = strtok_r(cmd, " ", &context);
	     word != NULL;
	     word = strtok_r(NULL, " ", &context)) {

		if (nargs >= MAXMENUARGS) {
			printf("Command line has too many words\n");
			return E2BIG;
		}
		args[nargs++] = word;
	}

	if (nargs==0) {
		return 0;
	}

	for (i=0; cmdtable[i].name; i++) {
		if (*cmdtable[i].name && !strcmp(args[0], cmdtable[i].name)) {
			assert(cmdtable[i].func!=NULL);
			result = cmdtable[i].func(nargs, args);
			return result;
		}
	}

	printf("%s: Command not found\n", args[0]);
	return EINVAL;
}

int main()
{
    pthread_t UI_thread, command_thread, executor_thread; /* Three concurrent threads */
    char *message0 = "User Interface";
    char *message1 = "Command Thread";
    char *message2 = "Executor Thread";
    int  iret0,iret1, iret2;

    /* Initilize count, two buffer pionters */
    count = 0; 
    buf_head = 0;  
    buf_tail = 0; 

    /* Create two independent threads:command and executors */
    iret0 = pthread_create(&UI_thread, NULL, UI, (void*) message0);
    iret1 = pthread_create(&command_thread, NULL, commandline, (void*) message1);
    iret2 = pthread_create(&executor_thread, NULL, executor, (void*) message2);

    /* Initialize the lock the two condition variables */
    pthread_mutex_init(&cmd_queue_lock, NULL);
    pthread_mutex_init(&cmd_queue_run_lock, NULL);
    pthread_cond_init(&cmd_buf_not_full, NULL);
    pthread_cond_init(&cmd_buf_not_empty, NULL);
    pthread_cond_init(&job_submit, NULL);
     
    pthread_join(command_thread, NULL);
    pthread_join(executor_thread, NULL);
    pthread_join(UI_thread, NULL); 

    return 0;
 }
