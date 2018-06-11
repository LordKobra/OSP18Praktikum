#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "ult.h"

unsigned int amountOfBytes;	

static void threadA()
{	
	printf("heyho");
	amountOfBytes = 0;
	char curr;
	int fdR;
	int fdW;
	int statistic
	if((fdR = open("/dev/random", O_RDONLY) < 0){
		fprintf(stderr, "reason for failure in threadA fdR: %s\n" , strerror(errno));
		exit(-1);	
	}
	if((fdW = open("/dev/null" , O_WRONLY) < 0){
		fprintf(stderr, "reason for failure in threadA fdW: %s\n", strerror(errno));
		exit(-1);
	}
	while(1){
		ult_read(fdR , &curr , 1);
		if(write(fdW, &curr, 1) == -1){
			fprintf(stderr, "problems in write threadA: %s\n", strerror(errno));
			exit(-1);
		}
		amountOfBytes++;

	}	
	ult_exit(0);
}

static void threadB()
{
	printf("heyob\n");
	ult_exit(0);
}

static void myInit()
{
	int tids[2], i, status;
	
	printf("spawn A\n");
	tids[0] = ult_spawn(threadA);
	printf("spawn B\n");
	tids[1] = ult_spawn(threadB);
	
	for (i = 0; i < 2; ++i)
	{
		printf("waiting for tids[%d] = %d\n", i, tids[i]);
		fflush(stdout);
		
		if (ult_join(tids[i], &status) < 0)
		{
			fprintf(stderr, "join for %d failed\n", tids[i]);
			ult_exit(-1);
		}
		
		printf("(status = %d)\n", status);
	}

	ult_exit(0);
}

int main()
{
	ult_init(myInit);
	return 0;
}
