/*

time calculation may be a nightware! 
Beware of float, int, unsigned int conversion.
you could use gettimeofday(...) to get down to microseconds!

*/

#include <pthread.h> 
#include <sys/mman.h> 
#include <stdio.h> 
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXFLOW 1234

typedef struct _flow{
    float arrivalTime ;
    float transTime ;
    int priority ;
    int id ;
} flow;

flow flowList[MAXFLOW];   // parse input in an array of flow
flow *queueList[MAXFLOW];  // store waiting flows while transmission pipe is occupied.
pthread_t thrList[MAXFLOW]; // each thread executes one flow
pthread_mutex_t trans_mtx = PTHREAD_MUTEX_INITIALIZER ; 
pthread_cond_t trans_cvar = PTHREAD_COND_INITIALIZER ;  

int numFlows;
int queueSize = 0;
int myPipe = 0;
//flow *trans_item; //current transimition item

struct timeval startTime;

double getmyTime(){	//total elapsing time for a flow
	struct timeval timev;
	gettimeofday(&timev, NULL);
	
	double totalTime = (timev.tv_sec - startTime.tv_sec) * 1.0;
	totalTime += (timev.tv_usec - startTime.tv_usec) / 1000000.0; // Microseconds to seconds

	return totalTime;
}

void requestPipe(flow *item) {
	flow *tmp;
	int i = queueSize;
	int k;
	
	pthread_mutex_lock(&trans_mtx); //lock mutex;		
	if (myPipe == 0 && queueList[0]==NULL) {
		//if()        	
		myPipe = item->id;
        	pthread_mutex_unlock(&trans_mtx); 
		return ;
	}else{	
		pthread_mutex_unlock(&trans_mtx);
		//wait till pipe to be available and be at the top of the queue  
		//pthread_mutex_lock(&trans_mtx);
		queueList[queueSize] = item;	//add in queueList
		//printf("queyeList flow id:%d ,number:%d \n", queueList[queueSize]->id,queueList[queueSize]);
		while(i > 0){
			//printf("------------compare---------\n");
			//priorty check if arrival time is same
			if(queueList[i-1]->priority > queueList[i]->priority){
				tmp = queueList[i-1];
				queueList[i-1] = queueList[i];
				queueList[i] = tmp;
			}else if(queueList[i-1]->priority == queueList[i]->priority){
				//start arrival time check		
				if(queueList[i-1]->arrivalTime > queueList[i]->arrivalTime){
						tmp = queueList[i-1];
						queueList[i-1] = queueList[i];
						queueList[i] = tmp;
				}else if(queueList[i-1]->arrivalTime == queueList[i]->arrivalTime){
					if(queueList[i-1]->transTime > queueList[i]->transTime){
						tmp = queueList[i-1];
						queueList[i-1] = queueList[i];
						queueList[i] = tmp;
					}else if(queueList[i-1]->transTime == queueList[i]->transTime){
						//start id check
						if(queueList[i-1]->id > queueList[i]->id){
							tmp = queueList[i-1];
							queueList[i-1] = queueList[i];
							queueList[i] = tmp;
						}
					}
				}
			}
			i--;
		} //end while
		queueSize++;
		printf("Flow %2d waits for the finish of flow %2d. \n", item->id, myPipe);
		
		//printf("queueList after softing:%d,%d,%d,%d,%d  \n",queueList[0],queueList[1],queueList[2],queueList[3],queueList[4] );
				
		
		//check if there is already a flow transmitting, wait to be signaled so the first item in queueList can run
		pthread_mutex_lock(&trans_mtx);			
		while (myPipe != item->id){
			pthread_cond_wait(&trans_cvar, &trans_mtx);
			myPipe = queueList[0]->id;
			
		
		}
		int j;
		//move the rest of the queue down one spot
		for (j = 0; j < numFlows; j ++){
			queueList[j] = queueList[j+1];
		}
		queueSize --;
		
	}
		//pthread_mutex_unlock(&trans_mtx);
    	 //unlock mutex;
	return;
}



void releasePipe(flow *item) {
	pthread_mutex_unlock(&trans_mtx);
	// Signals the scheduler that another flow can transmit 

	pthread_cond_broadcast(&trans_cvar);
	//printf("========send signal===========\n");	

	
	//myPipe = 0;
	
	return;
}

// entry point for each thread created
void *thrFunction(void *flowItem) {

    	flow *item = (flow *)flowItem ;

	//myPipe = 0;
	
    	// wait for arrival
    	usleep(item->arrivalTime * 1000000);
   	printf("Flow %2d arrives: arrival time (%.2f), transmission time (%.1f), priority (%2d). \n", item->id, getmyTime(),  item->transTime, item->priority);
	
    	requestPipe(item) ;
	//printf("trans_item : %d \n", myPipe);
    	printf("Flow %2d starts its transmission at time %.2f. \n", item->id, getmyTime());

    	// sleep for transmission time
    	usleep(item->transTime * 1000000);

    	releasePipe(item) ;
    	printf("Flow %2d finishes its transmission at time %d. \n", item->id,  (int)getmyTime());
	
	pthread_mutex_unlock(&trans_mtx);	
}


int main(int argc, char* argv[]) {
	FILE     *fp;
   	//char path[256];
	int i, j;
	gettimeofday(&startTime, NULL);

	if (argc != 2) {
		printf("\nPlease enter a 2-parameter command line\n");
		return 0;
	} else {
    		// file handling
    		fp = fopen(argv[1], "r");

    		if(fp == NULL) {
			printf("Could not open file\n");
			return 0;
		}
			
		// the first item must be an integer(number of flows)
		if(fscanf(fp, "%d", &numFlows) != 1){
			return 0;
		
		}
		//printf("numf%d:\n",numFlows);
		for (i = 0; i < numFlows; i++){
        		//parse line, using strtok() & atoi(), store them in flowList[i] ;			
			//myFlows = malloc(numFlows * sizeof(flow));
			if (fscanf(fp, "%d:%f,%f,%d", &flowList[i].id, &flowList[i].arrivalTime, &flowList[i].transTime, &flowList[i].priority) != 4){
				return 0;				
			}	
			// set time in seconds
			flowList[i].arrivalTime /= 10;
			flowList[i].transTime /= 10;
			//printf("id: %d, arrivalTime: %.2f, transTime: %.2f, priority: %d \n", flowList[i].id, flowList[i].arrivalTime, flowList[i].transTime, flowList[i].priority);		
		}

		fclose(fp);	 // release file descriptor
	 	for(i =0 ; i<numFlows; i++){
			//printf("bkser\n");
        		// create a thread for each flow 
        		pthread_create(&thrList[i], NULL, thrFunction, (void *)&flowList[i]) ;
		}

		for(j =0 ; j<numFlows; j++){
			pthread_join(thrList[j], NULL); // wait for all threads to terminate
		}	
		// destroy mutex & condition variable
		pthread_mutex_destroy(&trans_mtx);
		pthread_cond_destroy(&trans_cvar);
		//sleep(10);	//wait for customers leaving
		
	}
	return 0;
} 



