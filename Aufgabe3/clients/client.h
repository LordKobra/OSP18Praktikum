
#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED
#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define PORT 9000
#define HOST "127.0.0.1"
#define ECHO



static void die(const char* msg){
 fputs(msg, stderr);
 putc('\n', stderr);
 exit(-1);
}


int  ctrlC = 0; // was ctrl c pressed recently?

void  INThandler(int sig)
{
	ctrlC = 1;
 	return;
}

char dir[512]; // updated every time cd is called

void directory(char buf[]){              //rueckgabe von actuellen "ort"(also einfach pwd)
	int pid;
	int pipefd[2];

	if(pipe(pipefd) != 0){
	fprintf(stderr, "upsiiiiii");
	exit(-1);	
	}
	if((pid=fork()) == 0){ //this is the child herleluyer
		close(pipefd[0]);
		dup2(pipefd[1], 1);
		close(pipefd[1]);
		execl("/bin/pwd", "pwd", NULL);
		fprintf(stderr, "didnt work :(\n");
		exit(-1);	
	}

	close(pipefd[1]);
	if(read(pipefd[0], buf, 512) <=0){                //ist falsch pls edit
		fprintf(stderr, "its not true, thats impossible\n");
		exit(-3);		
	}
	close(pipefd[0]);
	return;
}

char* readLine()
{
	char *line = NULL;
	size_t buffersize = 0;
	ssize_t check = getline(&line, &buffersize, stdin);
	if(check == EOF){
		return NULL;	
	}
	return line;
}

char** parse(char* line){
	char ** ans = malloc( 64 * sizeof(char*));	
	char* word;
	int counter = 0;
	word = strtok(line, " \r\n");   // check if words are seperated by space, /n, or /r
	while(word != NULL){
		ans[counter++] = word;
		word = strtok(NULL, " \r\n"); 
		
	}
	ans[counter] = NULL;
	return ans;
}

#endif
