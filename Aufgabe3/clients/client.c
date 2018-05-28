#include "client.h"

int client;


int execute(char** args){
/* the READ command does not work like we wanted, what is wrong there?*/
//put
	if (strcmp(args[0],"put") == 0){

		FILE *fp = fopen(args[1], "r");
		int rval;
		char usrBuf[256];
		memset(usrBuf, 0, sizeof(usrBuf));
		if (fp) {
			while ((rval = fread(usrBuf, 1, sizeof(usrBuf), fp)) > 0) {
				if (write(client, usrBuf, sizeof(usrBuf)) < 0)
					die("Couldn't send file"); 
			}
			printf("send %s", args[1]);
			fclose(fp);
		} else {
			printf("file not found\n");
		}
//get
	} else if (strcmp(args[0],"get") == 0){

		
		FILE *fp = fopen(args[1], "w");
		ssize_t rval;
		char srvBuf[256];
		memset(srvBuf, 0, sizeof(srvBuf));
		if (write(client, args, sizeof(args)) < 0)
			die("Couldn't send message"); 
		if(fp){
		    while((rval = read(client, srvBuf, 0)) != 0){
				fwrite(srvBuf, 1, rval, fp);
				memset(srvBuf, 0, 256*sizeof(char));
		    }
		    printf("received %s", args[1]);
		    fclose(fp);
	    	} else {
			printf("file not found\n");
		}
	//exit	
	} else if(strcmp(args[0],"exit") == 0) {
		if (write(client, args, sizeof(args)) < 0)
			die("Couldn't send message");
		return 0;
	//rest
	} else {
		char srvBuf[256];
		memset(srvBuf, 0, sizeof(srvBuf));
		if (write(client, args, sizeof(args)) < 0)
			die("Couldn't send message"); 
		if(read(client, srvBuf, 5) < 0)
			die("Couldn't read message");
		
		printf("[server]:%s \n", srvBuf);
	}
return 1;
	
}

int chkUsrInp(){
	char* line;
	char** args;
	int status = 1;	
	do{
			//read command  (CHECK)
			line = readLine();
			if (line != NULL){
		
			//parse input -> command and arguments (CHECK)
			args = parse(line);
			//exec command   
			status = execute(args);
			
			//free mem
			free(line);
			free(args);
		}	
	}while(status);	
	return status;
}

int main()
{
	struct sockaddr_in addr = {
	.sin_family = AF_INET,
	.sin_port =htons(PORT),
	.sin_addr.s_addr = inet_addr(HOST)
	};

	if(( client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("Couldn't open the socket");

	if (connect(client, (struct sockaddr*) & addr, sizeof(addr)) < 0 )
		die("Couldn't connect to socket");
	//reading user until exit
	chkUsrInp();
	printf("Client Exit\n");
	
	close(client);	
	return 0;
}
