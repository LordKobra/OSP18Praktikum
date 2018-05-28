#include "client.h"

int client;


int execute(char** args){
	printf("hi leute\n\n\n");
	if (strcmp(args[0],"put") == 0){

		FILE *fp = fopen(args[1], "r");
		int nread;
		char lineBuf[256];
		if (fp) {
			while ((nread = fread(lineBuf, 1, sizeof(lineBuf), fp)) > 0) {
				if (write(client, lineBuf, sizeof(lineBuf)) < 0)
					die("Couldn't send file"); 
			}
		} else {
			printf("file not found\n");
		}
		fclose(fp);
	}/* else if (strcmp(args[0],"get") == 0){
	    FILE *fp;
	    int rval;
	    fp = fopen(filename, "w");
	    char buf[256];
	    while((rval = read(client, buf, sizeof(buf))) > 0)
		fwrite(buf, 1, rval, fp);
	    fclose(fp);
	    printf("received %s", filename);

	} */else {
		if (write(client, args, sizeof(args)) < 0)
			die("Couldn't send message");
	}
return 1;
	
}

int chkUsrInp(){
	char* line;
	char** args;
	int status = 1; //of command
	//print directory  (CHECK)
	//directory(dir);	
	do{
		//printf("~%s/> ", strtok(dir,"\r\n"));
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
void chkSvrInp(){
	int bytes;
	char srvBuf[256];
while((bytes=read(client,srvBuf,sizeof(srvBuf)))!=0){
if(bytes < 0)
die("Couldn't recieve message");
printf("%s", srvBuf);
};
return;
}

void usershell()
{
	//int cancel;
	while(1){
	chkSvrInp();
	chkUsrInp();
}
}

int main()
{
	struct sockaddr_in addr = {
	.sin_family = AF_INET,
	.sin_port =htons(PORT),
	.sin_addr.s_addr = inet_addr(HOST)
	};
	//char buf[256];
	//int client;
	if(( client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("Couldn't open the socket");

	if (connect(client, (struct sockaddr*) & addr, sizeof(addr)) < 0 )
		die("Couldn't connect to socket");

	usershell();
/*/TEST CD
	if (write(client, "cd ..", 5) < 0)
		die("Couldn't send message");
	if(read(client, buf, sizeof(buf)) < 0)
		die("Couldn't read message");
	printf("[recv] %s \n", buf);
*/
	printf("Client Exit\n");
	
	close(client);	
	return 0;
}
