#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


pthread_t* thread;
int coll;
int* waiting;
pthread_mutex_t* active;
double dur;
int fun;
void* (*collectorp)(void* val);

unsigned int* randChoice(void* point);
int circle(int origin, int offset);
int checkKonto(int self, int check);
int checkAmount(int check);
void collect(int self, int victim);
void day(int self);
void* collector(void* val);
void exec();

struct CollectorInfo {
	int Eingangsbuchungen;
	int Ausgangsbuchungen;
	int Kontostand;
} ;

struct CollectorInfo* kontos;


unsigned int* randChoice(void* point){
	unsigned int* ans = point;
	*ans = time(NULL) ^ getpid() ^ pthread_self();
	int temp = rand_r(ans);	
	*ans = temp; 
	return ans;
}


int circle(int origin, int offset){
	int curr = offset;
	while( waiting[curr] != -1){		
		curr = waiting[curr];
		if(curr == origin){
			return 0;
		}
	}
	return 1;
}

int checkKonto(int self, int check){
	if(kontos[check].Kontostand < 200){
		if(circle(self, check)){
			return 0;
		}else{
			return 1;
		}
	}else{
		return 1;	
	}
}

int checkAmount(int check){
	if(kontos[check].Kontostand < 200)
		return 0;
	return 1;

}

void collect(int self, int victim){
	//printf("%i collecting from %i, with %i amount of money\n", self, victim, kontos[victim].Kontostand);
	while(!checkAmount(victim)){
		waiting[self] = victim;
		if(circle(self, victim)){
				day(self);   //to avoid deadlock
		}

	}
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	pthread_mutex_lock(active+victim);
	waiting[self]= -1;
	int temp = kontos[victim].Kontostand/2;
	kontos[victim].Kontostand -= temp;
	kontos[victim].Ausgangsbuchungen ++;
	pthread_mutex_unlock(active+victim);
	pthread_mutex_lock(active+self);
	kontos[self].Kontostand += temp;
	kontos[self].Eingangsbuchungen ++;
	pthread_mutex_unlock(active+self);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
}


void day(int self){
	int temp = 0;	
	int *stuff = &temp;
	while(1){	
		do{	
			temp = (*randChoice(stuff)) % coll ;
			//printf("%i : %i inside\n", temp, (int)pthread_self());
		}while(pthread_equal(pthread_self(), thread[temp]) || !checkKonto(self, temp));
		//printf("%i : %i\n", temp, self);		
		collect(self, temp);
		sched_yield();
	}
	return;
		
}


void* collector (void *val){	
	int self = 0;
	while(!pthread_equal(thread[self], pthread_self())){
		self++;
		if(self == coll){
			fprintf(stderr,"thread apparantly is not in the thread array\n");
			exit(-1);
		}
	}
	kontos[self].Eingangsbuchungen = 0;
	kontos[self].Ausgangsbuchungen = 0;
	kontos[self].Kontostand = fun;
	waiting[self] = -1;
	day(self);
	return NULL;
}


void exec(){
	waiting = calloc( (size_t) coll, (size_t) coll * sizeof(int));
	kontos = malloc(coll * sizeof( struct CollectorInfo)); 
	thread = malloc(coll * sizeof(pthread_t));
	active = malloc(coll * sizeof(pthread_mutex_t));
	for(int i = 0; i< coll; i++){
		pthread_mutex_init(active+i , NULL);
		pthread_mutex_lock(active+i);
	}
	collectorp = &collector;
	for(int i = 0;i < coll; i++){
		if(pthread_create(thread+i, NULL, collectorp, NULL) != 0){
			fprintf(stderr, "pthreadcreate() failed");
			exit(-1);
		}
		
	}
	
	for(int i = 0; i< coll; i++){
		pthread_mutex_unlock(active+i);
	}

	sleep (dur);
	
	int total = 0;
	int totalEingangsbuchungen = 0;
	int totalAusgangsbuchungen = 0;	
	for(int i = 0; i < coll ; i++){
		pthread_cancel(thread[i]);	
	}
	
	for(int i =0; i < coll; i++){
		total += kontos[i].Kontostand;
		totalEingangsbuchungen += kontos[i].Eingangsbuchungen;
		totalAusgangsbuchungen += kontos[i].Ausgangsbuchungen;
		printf("thread%i : Kontostand = %i , Eingangsbuchungen = %i , Ausgangsbuchungen = %i \n", i, kontos[i].Kontostand , kontos[i].Eingangsbuchungen, kontos[i].Ausgangsbuchungen);
	}
	
	printf("insgesamt : Kontostand = %i , Eingangsbuchungen = %i , Ausgangsbuchungen = %i \n", total, totalEingangsbuchungen, totalAusgangsbuchungen);
	
	return;
}





int main(int argc, const char* argv[])
{
	dur = 2; // default duration in seconds
	coll = 5;  // default number of tax collectors
	fun = 300;     // default funding per collector in Euro
	
	// allow overriding the defaults by the command line arguments
	switch (argc)
	{
	case 4:
		dur = atof(argv[3]);
		/* fall through */
	case 3:
		fun = atoi(argv[2]);
		/* fall through */
	case 2:
		coll = atoi(argv[1]);
		/* fall through */
	case 1:
		/*printf(
			"Tax Collectors:  %d\n"
			"Initial funding: %d EUR\n"
			"Duration:        %g s\n",
			collectors, funds, duration
		);*/
		break;
		
	default:
		printf("Usage: %s [collectors [funds [duration]]]\n", argv[0]);
		return -1;
	}
	
	// TODO: implement the scenario	
	exec();
	return 0;
}
