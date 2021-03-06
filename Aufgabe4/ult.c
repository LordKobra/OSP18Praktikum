#include "ult.h"
#include "array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>

#define _XOPEN_SOURCE
#include <ucontext.h>

/* thread control block */

int current_free;

typedef struct tcb_s
{	
	char mem[THREAD_STACK_SIZE];
	int active;
	int status;
	int ID;
	int joiner;
	ucontext_t context;
	int fd;
	/* data needed to restore the context */
	
} tcb_t;

int current_active;
tcb_t* threeds;
int threeds_size;
fd_set set;
int fd_amount;

void ult_init(ult_f f)
{	
	tcb_t top;
	threeds = ( tcb_t*) malloc(10*sizeof(tcb_t));
	threeds_size = 10;
	current_free = 0;
	fd_amount = 0;
	if(getcontext(&top.context) != 0){
		fprintf(stderr, "in ult_init getcontext failed");
		exit(-1);
	}
	top.context.uc_stack.ss_flags =0;
	top.context.uc_stack.ss_sp = top.mem;
	top.context.uc_stack.ss_size = THREAD_STACK_SIZE;
	top.ID = current_free;
	if(top.context.uc_stack.ss_sp == NULL){
		fprintf(stderr,"mem array was not initialized correctly");
		exit(-1);
	}
	makecontext(&top.context, f, 0);
	threeds[current_free++] = top;
	current_active = 0;
	setcontext(&top.context);
	fprintf(stderr, "in ult_exit setcontext should not return here\n");
	return;
	
}

int ult_spawn(ult_f f)
{	
	tcb_t spawned;
	if(getcontext(&spawned.context) != 0){
		fprintf(stderr, "in ult_spawn getcontext failed");
		exit(-1);	
	}
	spawned.context.uc_stack.ss_flags = 0;
	spawned.context.uc_stack.ss_sp = spawned.mem;
	spawned.context.uc_stack.ss_size = THREAD_STACK_SIZE;
	spawned.ID = current_free;
	spawned.active = 1;
	spawned.fd = -1;
	makecontext(&spawned.context, f, 0);
	if(current_free + 1 == threeds_size){
		threeds = (tcb_t*)realloc(threeds, (threeds_size + 10)*sizeof(tcb_t));
		threeds_size+=10;
	}
	threeds[current_free++] = spawned;
	return spawned.ID;
		
}

void ult_yield()
{
	return;
	int next = current_active + 1;
	if(next == threeds_size){
		next = 1;
	}
	int counter = 0;
	struct timeval time;
	time.tv_sec = 0;
	time.tv_usec = 100;
	fd_set open ;
	while(counter < threeds_size -2){
		open =set;
		if(select(fd_amount, &open, NULL, NULL , &time) == -1){
			fprintf(stderr, "reason of malfuction of select in yield: %s\n",strerror(errno));
			exit(-1);	
		} 
		if(threeds[next].active == 1){
			if(threeds[next].fd != -1){
				if(FD_ISSET(threeds[next].fd, &open) != 0){
					if(swapcontext(&threeds[current_active].context, &threeds[next].context) == -1){
					fprintf(stderr, "in yield swap has malfunctioned");
					exit(-1);
					}
				}
			}else if(swapcontext(&threeds[current_active].context, &threeds[next].context) == -1){
				fprintf(stderr, "in yield swap has malfunctioned");
				exit(-1);
			}
			return;
		}
		if(++next == threeds_size)
			next = 1;
		
	}
	return;
	
}

void ult_exit(int status)
{
	threeds[current_active].status = status;
	threeds[current_active].active = -1;
	if(current_active == 0){
		free(threeds);
		exit(0);
	}
	int goBackTo = threeds[current_active].joiner;
	setcontext(&threeds[goBackTo].context);
	fprintf(stderr, "in ult_exit setcontext should not return here\n");
	return;
}

int ult_join(int tid, int* status)
{	
	if(threeds[tid].active == -1){
		*status = threeds[tid].status;
		return 0;	
	}if(threeds[tid].active == 1){
		threeds[tid].joiner = current_active;
		int former_active = current_active;
		current_active = tid;
		if(swapcontext(&threeds[former_active].context, &threeds[tid].context) == -1){
			fprintf(stderr, "reason of failure in ult_join: %s\n", strerror(errno));  			
			return -1;
		}
		current_active = former_active;
		return 0;
	}	
	return -1;
}

ssize_t ult_read(int fd, void* buf, size_t size)
{
	char curr;
	char * ans = (char*)malloc(10);
	int curr_pos = 0;
	int buf_size = 10;
	threeds[current_active].fd = fd;
	FD_SET(fd, &set);
	fd_amount++;
	ult_yield();
	while(read(fd, &curr, 1) > 0){
		if((curr_pos + 1) == buf_size){
			buf = realloc(buf, buf_size + 10);
			buf_size += 10;		
		} 
		ans[curr_pos++] = curr;
		
	}
	
	buf = ans;
	return (ssize_t)curr_pos;
}
