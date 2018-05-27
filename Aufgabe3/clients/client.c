#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include "client.h"

#define PORT 9000
#define HOST "127.0.0.1"

static void die(const char* msg){
 fputs(msg, stderr);
 putc('\n', stderr);
 exit(-1);
}

int main()
{
	struct sockaddr_in addr = {
	.sin_family = AF_INET,
	.sin_port =htons(PORT),
	.sin_addr.s_addr = inet_addr(HOST)
	};
	char buf[256];
	int client;
	if(( client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("Couldn't open the socket");

	if (connect(client, (struct sockaddr*) & addr, sizeof(addr)) < 0 )
		die("Couldn't connect to socket");

//TEST CD
	if (write(client, "cd ..", 5) < 0)
		die("Couldn't send message");
	if(read(client, buf, sizeof(buf)) < 0)
		die("Couldn't read message");
	printf("[recv] %s \n", buf);
//TEST EXIT	
	if (write(client, "exit", 5) < 0)
		die("Couldn't send message");
	if(read(client, buf, sizeof(buf)) < 0)
		die("Couldn't read message");
	printf("[recv] %s \n", buf);

	printf("Client Exit\n");
	close(client);
	return 0;
}
