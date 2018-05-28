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


int main()
{
    struct sockaddr_in srv_addr;
    socklen_t sad_sz = sizeof(struct sockaddr_in);
    int opt = True;                         //opt for setsocketopt option
    int mSfd, addresslen;
    //int clientServer[30], maxClients = 30;                 // change to pointer
    //unsigned int csSize = 0;               //for size of clientServer when clientServer is a int*
    //ssize_t bytes;
    char buf[256];
    //fd_set readables;
    pid_t pid;


    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(PORT);
    srv_addr.sin_addr.s_addr = INADDR_ANY;

    if ((mSfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) //socket()
        die("Couldn't open the socket");

    if ((setsockopt(mSfd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt))) < 0 )    //accept multiple connections
        die("(setsockopt) multiple connection failed");


    if (bind(mSfd, (struct sockaddr*) &srv_addr, sad_sz) < 0)   //bind()
        die("Couldn't bind socket");

    if (listen(mSfd, SOMAXCONN) < 0)
        die("Couldn't listen to the socket");       //listen()

    /*for(int i = 0; i < maxClients; i++){
        clientServer[i] = 0;                    //no client servers as of yet
    }*/

    addresslen = sizeof(srv_addr);

    while(True){

        /* while true check for incoming clients
        if there then fork() and let child deal with input and output
        */
        //wait for something to happen
        //FD_ZERO(&readables);
        //FD_SET(mSfd, &readables);
	
        //activity = select( 1 , &readables, NULL, NULL, NULL);
printf("Ich bin's, der Server\n\n\n");
       /* if((activity < 0) && (errno != EINTR)){
            die("select error in while Loop");
        }*/

       // if(FD_ISSET(mSfd, &readables)){    //something happend with mSfd implies new incoming connection

            if((incoming = accept( mSfd, (struct sockaddr*) &srv_addr, (socklen_t*) &addresslen)) <0){
                die("accept in while loop/ something happend with mSfd");
            }

            if((pid = fork()) == 0){        //if child
                // accept inputs write outputs
                // in a while loop thatcwith select
                if(read(incoming, buf, sizeof(buf)) < 0){
                    die("problems with read in child");
                }

                char * line = NULL;
                char** args;
                int status = 1; //of command


                //print directory  (CHECK)
                directory(dir);
                do{

                    //printf("~%s/> ", strtok(dir,"\r\n"));

                        //read command  (CHECK)
                    strcpy( line, buf);
                    if (line != NULL){             //this part depends on client if client message includes \n this needs to be changed

                        //parse input -> command and arguments (CHECK)
                        args = parse(line);
                        //exec command
                        status = execute(args);


                        //free mem
                        free(line);
                        free(args);
                    }
                }while(status);
                close(incoming);
                _exit(EXIT_SUCCESS);          //terminate after client uses exit in shell

            //}
               // parent just waits for next connection
        }



    }


    /*
    cfd = accept(sfd, (struct sockaddr*) &cli_addr, &sad_sz);
    if (cfd < 0)
        die("Couldn't accept incoming connection");
    while ((bytes = read(cfd, buf, sizeof(buf))) != 0)
    {
        if (bytes < 0)
            die("Couldn't receive message");
        if (write(cfd, buf, bytes) < 0)
            die("Couldn't send message");
    }
    close(cfd);
    close(sfd);
    */
    close(mSfd);
    return 0;
} 
