#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "scheduler.h"
#include "dispatcher.h"

#define NUM_OF_CMD   5  /* The number of submitted jobs   */
#define CMD_BUF_SIZE 10 /* The size of the command queueu */
#define MAX_CMD_LEN  512 /* The longest commandline length */

pthread_mutex_t cmd_queue_lock;  /* Lock for critical sections */
pthread_mutex_t cmd_queue_run_lock; /*UI to scheduler mutex*/
pthread_cond_t cmd_buf_not_full; /* Condition variable for buf_not_full */
pthread_cond_t cmd_buf_not_empty; /* Condition variable for buf_not_empty */
pthread_cond_t job_submit;



unsigned int buf_head;
unsigned int buf_tail;
unsigned int count;
//unsigned int lock = 0;

char *param_list[3];
struct queue *cmd_buffer[CMD_BUF_SIZE];

struct queue
{
char name[10];
int burst_time;
int priority;
};


//Scheduler
void *commandline(void *ptr){
    char *message;
    struct queue *temp_cmd;
    unsigned int i;
    char num_str[8];
    size_t command_size;

    for (i = 0; i < NUM_OF_CMD; i++) {    
        pthread_mutex_lock(&cmd_queue_lock);
        while (count == CMD_BUF_SIZE) {
            pthread_cond_wait(&cmd_buf_not_full, &cmd_queue_lock);
        }
        pthread_mutex_unlock(&cmd_queue_lock);
        temp_cmd = malloc(MAX_CMD_LEN*sizeof(struct queue));

	while(lock == 0){
		pthread_cond_wait(&job_submit, &cmd_queue_run_lock);	
	}
	lock--;

        pthread_mutex_lock(&cmd_queue_lock); 
	strcpy(temp_cmd->name,param_list[0]);
	temp_cmd->burst_time = atoi(param_list[1]);
	temp_cmd->priority = atoi(param_list[2]);   
        cmd_buffer[buf_head] = temp_cmd;
        count++;
        /* Move buf_head forward, this is a circular queue */ 
        buf_head++;
        if (buf_head == CMD_BUF_SIZE)
            buf_head = 0;
	
        pthread_cond_signal(&cmd_buf_not_empty);  
        /* Unlock the shared command queue */
        pthread_mutex_unlock(&cmd_queue_lock);
    }
}
