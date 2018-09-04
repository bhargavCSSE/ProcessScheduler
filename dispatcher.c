#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "dispatcher.h"


#define NUM_OF_CMD   5  /* The number of submitted jobs   */
#define CMD_BUF_SIZE 10 /* The size of the command queueu */
#define MAX_CMD_LEN  512 /* The longest commandline length */

pthread_mutex_t cmd_queue_lock;  /* Lock for critical sections */
pthread_mutex_t cmd_queue_run_lock; /*UI to scheduler mutex*/
pthread_cond_t cmd_buf_not_full; /* Condition variable for buf_not_full */
pthread_cond_t cmd_buf_not_empty; /* Condition variable for buf_not_empty */
pthread_cond_t job_submit;



unsigned int buf_head;/* Global job queue head*/
unsigned int buf_tail;/*Global job queue tail*/
unsigned int count;/*Job count in the queue*/
unsigned int lock = 0;

int policy = 0;
struct queue *cmd_buffer[CMD_BUF_SIZE];

struct queue
{
char name[10];
int burst_time;
int priority;
};



//Dispatcher
void *executor(void *ptr){
    char *message;
    unsigned int i;
 	char *arg[5];
	pid_t pid;

    for (i = 0; i < NUM_OF_CMD; i++) {
        pthread_mutex_lock(&cmd_queue_lock);
        while (count == 0) {
            pthread_cond_wait(&cmd_buf_not_empty, &cmd_queue_lock);
        }

	if(policy == 0){
	count--;
	execv_call(cmd_buffer[buf_tail]);
	buf_tail++;
	}
	
	else if(policy == 1){
		policy_check();
		for(buf_tail=0; count!=0; buf_tail++){
		execv_call(cmd_buffer[buf_tail]);
		count--;
		}
	sleep(1);
	printf("Simulation of SJF finished\nPRESS q to quit\n");
	pthread_mutex_unlock(&cmd_queue_lock);
	}
   	
	if(policy == 2){
		policy_check();
		for(buf_tail=0; count!=0; buf_tail++){
		execv_call(cmd_buffer[buf_tail]);
		count--;
		}
	sleep(1);
	printf("Simulation of Priority finished\nPRESS h\n");
	pthread_mutex_unlock(&cmd_queue_lock);
	}

        if (buf_tail == CMD_BUF_SIZE)
            buf_tail = 0;

        pthread_cond_signal(&cmd_buf_not_full);
        /* Unlock the sharefd command queue */
        pthread_mutex_unlock(&cmd_queue_lock);
    } /* end for */
}
