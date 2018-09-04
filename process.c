#define BURST 0
#define PRIORITY 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
#include <string.h>
#include "dispatcher.h"


int main(int argc, char *argv[] )
{
  int i = 2;
  int a[2];
  char* msg[] = {"Burst time","Priority"};
  char jobname[20];
  strcpy(jobname, argv[1]);
  argc=argc-2;

  /* Print all the arguments passed from the commandline */
  while (argc > 0) {
    a[i-2] = atoi(argv[i]);
    i++;
    argc--;
  }
  //printf("Process %s is running\n",jobname);
  sleep(a[BURST]);
if(policy = 0)
  printf("Process %s with burst %d is finished\n",jobname,a[burst]);
  return 0;
}
