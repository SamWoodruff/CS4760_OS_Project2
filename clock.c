#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "clock.h"

int segmentId = 0;
struct times{
	int nanoseconds;
	int seconds;
};

struct times *timesPtr;

void createClock(){
	key_t mem_key = ftok(".",'a');		
	//allocate shared memory
	segmentId = shmget(mem_key, sizeof(struct times), IPC_CREAT | 0666);
	if(segmentId < 0){
		printf("Error in shmget");
		exit(1);
	}

	//attach shared memory
	timesPtr = (struct times*) shmat(segmentId, NULL ,0);
	if((intptr_t)timesPtr == -1){
		printf("Error in shmat");
		exit(1);
	}

	//start clock at time 0
	timesPtr->nanoseconds = 0;
	timesPtr->seconds = 0;
}

void unallocateClock(){
	//detach shared memory
	shmdt((void*)timesPtr);
	shmctl(segmentId, IPC_RMID, NULL);
}

void incrementClock(int incrementor){
	timesPtr->nanoseconds = timesPtr->nanoseconds + incrementor;
	if(timesPtr->nanoseconds % 1000000000 == 0){
		timesPtr->seconds = timesPtr->seconds + 1;
	}
}

unsigned int getNanoseconds(){
	return timesPtr->nanoseconds;
}

unsigned int getSeconds(){
	return timesPtr->seconds;
}
