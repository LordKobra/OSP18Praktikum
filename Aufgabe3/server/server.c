#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include "server.h"
#define PORT 9000
#define True 1
#define False 0



void remShell(){
	char buf[256];	
	char* output = NULL;
	char** args;
	int status = 1; //of command

	
	//print directory  (CHECK)
	directory(dir);	
	do{
		
		printf("hallo\n\n\n\n\n\n\n");
		sprintf(output, "~%s/> ", strtok(dir,"\r\n"));
		strcpy(buf, output);
		if(write(cfd, buf, sizeof(buf)) < 0){	
			die("could not send pwd");
		}
			//read command  (CHECK)S
		if(read(cfd, buf, 30)<0)
			die("could not read incoming message");
		if (buf != NULL){
				
			//parse input -> command and arguments (CHECK)
			args = parse(buf);
			//exec command   
			status = execute(args);
		
	
			//free mem
			
			free(args);
		}	
	}while(status);	
}


int main()
{
    struct sockaddr_in srv_addr, cli_addr;
	socklen_t sad_sz = sizeof(struct sockaddr_in);	
	int opt = 1;
	pid_t pid;
	
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(PORT);
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	
	if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("Couldn't open the socket");

	//socket can be reused
	if ((setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt))) < 0 )    
		die("(setsockopt) multiple connection failed");
	
	if (bind(sfd, (struct sockaddr*) &srv_addr, sad_sz) < 0){
		printf("%s \n", strerror(errno));	
		die("Couldn't bind socket");
	}
	
	if (listen(sfd, 1) < 0)
		die("Couldn't listen to the socket");
	while(True){
		if((cfd = accept(sfd, (struct sockaddr*) &cli_addr,&sad_sz))<0){
			die("could not accept incoming connection");
		}
		if((pid = fork()) < 0)
			die("fork did not work correctly");
		if(pid == 0){ // if child
			remShell();
		}
		
	}
	//close(cfd);
	close(sfd);
	return 0;
} 
