
#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

void INThandler(int sig);
static void die(const char* msg);
void directory(char buf[]);
char** parse(char* line);
int execute(char** args);
void cd(char *path);
void waitProcess(char ** args);
void prog(char **args);
int checkForElements(char** args);
void backGr(char** args);
void parExec(char** args1, char ** args2);
void normExec(char** args);
int redir(int fd, const char* file);
void put(char* filename, int clientid ); 
void get(char* filename, int clientid);




int  ctrlC = 0;

int sfd, cfd;  //the client which is different for every child process

char dir[512]; // updated every time cd is called




int redir(int fd, const char* file)
{
	int fd2 = open(file, O_WRONLY | O_CREAT | O_TRUNC , 0644);
	
	if (fd2 < 0)
		return -1;
	
	dup2(fd2, fd);
	close(fd2);
	
	return 0;
}

void  INThandler(int sig)
{
	ctrlC = 1;
 	return;
}

static void die(const char* msg){
	fputs(msg, stderr);
	putc('\n', stderr);
	exit(-1);
}


void directory(char buf[]){              //rueckgabe von actuellen "ort"(also einfach pwd)
	int pid;
	int pipefd[2];

	if(pipe(pipefd) != 0){
		//fprintf(stderr, "upsiiiiii");
		if(write(cfd, "directory did not work", 22) < 0){
			die("could not send error message in directory");
		} 
		exit(-1);	
	}
	if((pid=fork()) == 0){ //this is the child herleluyer
		close(pipefd[0]);
		dup2(pipefd[1], 1);
		close(pipefd[1]);
		execl("/bin/pwd", "pwd", NULL);
		//fprintf(stderr, "didnt work :(\n");
		if(write(cfd, "directory child did not work", 28) < 0){
			die("could not send error message in directory child");
		} 
		exit(-1);	
	}

	close(pipefd[1]);
	if(read(pipefd[0], buf, 512) <=0){                //ist falsch pls edit
		//fprintf(stderr, "its not true, thats impossible\n");
		if(write(cfd, "directory reading did not work", 30) < 0){
			die("could not send error message in directory parent");
		} 
		exit(-3);		
	}
	close(pipefd[0]);
	return;
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



int execute(char** args){
	
	if(args[0] == NULL)
		return 1;

	if (strcmp(args[0],"cd") == 0){
		if(args[2] != NULL){
			//fprintf(stderr, "cd did not work\n");
			if(write(cfd, "cd in execute did not work", 26) < 0){
				die("could not send error message in cd");
			} 
			return 1;
		}		
		cd(args[1]);
		return 1;
	}else if(strcmp(args[0],"wait")==0){
		waitProcess(args);
		return 1;
	}else if(strcmp(args[0],"exit")==0){
			return 0;	
	}
	prog(args);

	return 1;
	

	
}

void cd(char *path)
{
    char cwd[512] = { 0 };
    memcpy(cwd, dir, sizeof(cwd));
    char fwd[512] = { 0 };
    if(path == NULL){
        int i = 0, slash = 0;
        while(slash != 2){
            if(cwd[i] == '/') {
                 if (i == 0){
                    fwd[i] = cwd[i];
                    i++;
                }
                slash++; 
            }else{
                fwd[i] = cwd[i];
                i++;
            }    
        }
    }else if(strcmp(path, "..") == 0){
        int i = 511, slash = 0;
        while(slash != 1){
            if(cwd[i] == '/'){
                slash++;
                if(slash == 1){
                    memcpy(fwd, cwd, sizeof(char)*(i));
                }
                i--;
            }else{
                i--;
            }
        }
    }else if(path[0] == '/'){
        memcpy(fwd, path, sizeof(fwd));
    }else{
        memcpy(fwd, cwd, sizeof(fwd));
        strcat(fwd,"/"); 
        strcat(fwd,path);
    }
        
    errno = chdir(fwd);
    if (errno  == -1) {
        //fprintf(stderr, "Error changing directory\n");
	if(write(cfd, "Error changing directory", 24) < 0){
		die("error message in cd did not work correctly");
	} 
    }else{
        directory(dir);
    }
    return;
}


void waitProcess(char ** args){
    	
	ctrlC = 0;
    	int status = 0;
    	//int currentPID = 0;
    	int waited = 0;
    	int i = 1;
   	int pidcount = 0;
    if(args[1] == NULL){
        //printf("Error waiting on process: no process selected\n");
	if(write(cfd, "Error waiting on process: no process selected", 35) < 0){
		die("1st error message in waitProcess did not work correctly");
	}
        return;
    }
    while(args[i] != NULL && status == 0){
        if(atoi(args[i]) != 0){
            status=kill(atoi(args[i]), 0);
            if(status !=0){
             //printf("Error waiting on process:%s! %s\n",args[i], strerror(errno));
	    	char output[50];
		sprintf(output, "Error waiting on process:%s! %s", args[i], strerror(errno));
		if(write(cfd, output, sizeof(output))<0){
			die(" error message failed in waitProcess in while loop");
		}
            }
            i++;
        }else{ 
            //printf("Error waiting on process:%s! The process is no natural number\n",args[i]);
		char output[80];		
		sprintf(output,"Error waiting on process:%s! The process is no natural number", args[i]);
		if(write(cfd, output, sizeof(output))<0){
			die(" 2nd error message failed in waitProcess in while loop");
		}
            status = -1;
        }
    }
    pidcount = i - 1;
    if(pidcount <= 0) pidcount = 1;
    int child_pids[pidcount];
	int statusList[pidcount];
    memset(child_pids, 0, pidcount*sizeof(int));
    i = 1;
    if(status == 0){
        while(args[i] != NULL && i <=pidcount){
	        child_pids[i-1] = atoi(args[i]);
		i++;	
        }
        while(waited<pidcount){
            //interrupt
            if (signal(SIGINT, INThandler) == SIG_ERR){
                //fputs("signal-handler configured wrong\n", stderr);
		if(write(cfd,"signal-handler configured wrong" , 32 )<0){
			die("3rd error message failed in waitProcess in while loop");
		}
                return;
            }
            if(ctrlC == 1){
                //printf("\nforced 'wait' to return\n");
		if(write(cfd, "\n forced 'wait' to return", 26)<0){
			die(" 4th error message failed in waitProcess in while loop");
		} 
                return;
            }
            //pidcheck
            int j = 0;
            waited = 0;
            for(j = 0; j<pidcount; j++){
                waitpid(child_pids[j], &status, 0);
                if(WIFEXITED(status)){
			statusList[j] = status;                    
			waited++;
		}
            }

        }
        int k = 0;
        for(k = 0; k<pidcount; k++){
            //printf("%i terminated with Status %i \n", child_pids[k], WEXITSTATUS(statusList[k]));
		char output[50];
		sprintf(output, "%i terminated with Status %i \n", child_pids[k], WEXITSTATUS(statusList[k]));
		if(write(cfd, output, sizeof(output))<0){
			die("termination message in waitProcess did not work correctly");
		}
        }
    }
    return;
}





void prog(char **args){       //pipen fuer prog (bzw wenn nicht exit, wait oder cd)
	int check = checkForElements(args);
	if(strcmp(args[check], "&") == 0){
		args[check] = NULL;		
		backGr(args);
	}else if(strcmp(args[check], "|") == 0){
		char ** args1 = malloc(64 * sizeof(char*));
		char ** args2 = malloc(64 * sizeof(char*));
		int curr = 0;
		int posTwo = 0;

		for(int i = 0; i < check; i++){
			args1[i] = args[i];	
			curr++;	
		}

		curr++;    //skip |
	
		while(args[curr] != NULL){
			args2[posTwo++] = args[curr++];
		}
		
		parExec(args1, args2);
		free(args1);
		free(args2);
	}else if(strcmp(args[0], "put")){
		put(args[0], cfd);

	
	}else if(strcmp(args[0], "get")) {

		get(args[1], cfd);

	}else{
		normExec(args);
	}

	return ;
	
}


void get(char* filename, int clientid){
	FILE *fp = fopen(filename, "r");
	int nread;
	char* output = NULL;
	char buf[256];
	if (fp) {
    		while ((nread = fread(buf, 1, sizeof(buf), fp)) > 0)
        		if (write(cfd, buf, sizeof(buf)) < 0)
				die("Couldn't send file");
	} else{
		printf("file not found\n");
	}	
	fclose(fp);
	sprintf(output,"sent %s", filename);
	if(write(cfd, output, 20)<0){
		die("write in get did not function cerrectly");
	}
}



void put(char* filename, int clientid ){
	FILE *fp;
	int rval;
	char * output = NULL;
	fp = fopen(filename, "w");
	char buf[256];
	while((rval = read(cfd, buf, sizeof(buf))) > 0)
		fwrite(buf, 1, rval, fp);
	fclose(fp);
	sprintf(output,"received %s", filename);
	if(write(cfd, output, 20)<0){
		die("write in put did not function correctly");
	}
}



int checkForElements(char** args){    // & pos and | / pos = 0 is not defined 
	int pos = 0;
	int counter = 0;	
	while(args[counter]!= NULL){
		if(strcmp(args[counter], "&") == 0)
			pos = counter;
		if(strcmp(args[counter], "|") == 0)
			pos = counter;
		counter++;
	}
	return pos;
}


void backGr(char** args){      // for the & command
	pid_t pid;


	if((pid = fork())==0){ // if child
        	/*if(setpgid(pid, 0) != 0){
            		fprintf(stderr, "process group did not work in backGr");
            		exit(-1);
        	}*/
		setvbuf(stdout, NULL, _IONBF, 0);  //so prog doenst print in stdout : from here


		if (redir(1, "log.txt") < 0)
		{
			//fputs("Fehler beim Umleiten von stdout\n", stderr)			
			if(write(cfd, "Fehler beim Umleiten von stdout\n", 40)<0){
				die("1st error message in backGr did not function correctly");
			}
			exit(-1);
		}                                                      //to here
	
		execv(args[0], args);
		//fprintf(stderr, "\nfork in backGr did not work\n");
		if(write(cfd, "\nfork in backGr did not work\n", 33) < 0){
			die("2nd error message in backGr did not function correctly");
		}
		exit(-1);
	}
	printf("[%i]\n", pid);  // parent
}


void parExec(char** args1, char ** args2){	
	pid_t pid [2];
	int pipefd[2];
	if(pipe(pipefd) != 0){
		//fprintf(stderr, "pipe did not work in parExec\n");
		if(write(cfd, "pipe did not work in parExec\n", 40)<0){
			die("1st error message in parExec did not function correctly");
		}
		exit(-1);
	}

	if((pid[0] = fork()) == 0){ //first child
		// disable buffering for stdout
		setvbuf(stdout, NULL, _IONBF, 0);
	
		// redirect stdout to write into a file
		if (redir(1, "log.txt") < 0)
		{
			//fputs("Fehler beim Umleiten von stdout\n", stderr);
			if(write(cfd, "Fehler beim Umleiten von stdout\n", 40) < 0){
				die("2nd error message in parExec did not function correctly");
			}
			exit(-1);
		}
		close(pipefd[0]);
		dup2(pipefd[1], 1);
		close(pipefd[1]);		
		execv(args1[0], args1);
		//fprintf(stderr, "first fork in parExec did not work\n");
		if(write(cfd, "first fork in parExec did not work\n", 40) <0){
			die("3rd error message in parExe did not function correctly");
		}
		exit(-1); 	
	}

	if((pid[1] = fork()) ==0){ // second child
		// disable buffering for stdout
		setvbuf(stdout, NULL, _IONBF, 0);
	
		// redirect stdout to write into a file
		if (redir(1, "log.txt") < 0)
		{
			//fputs("Fehler beim Umleiten von stdout\n", stderr);
		if(write(cfd, "Fehler beim Umleiten von stdout\n", 40) <0){
			die("4th error message in parExe did not function correctly");
		}			
			exit(-1);
		}		

		close(pipefd[1]);
		dup2(pipefd[0], 0);
		close(pipefd[0]);				
		execv(args2[0], args2);
		//fprintf(stderr,"second fork in parExec did not work\n");
		if(write(cfd, "second fork in parExec did not work\n", 40) <0){
			die("5th error message in parExe did not function correctly");
		}		
		exit(-1);
	}

	close(pipefd[0]);     //parent
	close(pipefd[1]);
	return;
	
}



void normExec(char** args){
	
	pid_t pid;
	int status = 0;
	
	if((pid = fork())==0){ //if child
		execv(args[0], args);
		//fprintf(stderr, "fork in normExec did not work\n");
		if(write(cfd, "fork in normExec did not work\n", 30) < 0){
			die("1st error message in normExec did not function correctly");
		}
		exit(-1);
	}
	//parent
	waitpid( pid, &status, 0);
	return;
}



#endif
