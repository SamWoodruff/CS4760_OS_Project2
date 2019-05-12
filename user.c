#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
struct times{
	int nanoseconds;
	int seconds;
};
struct times *timesPtr;

int main(int argc, char *argv[]){	
	int segmentId;
	unsigned int *nanoseconds;
	key_t mem_key = ftok(".",'a');		
	
	//allocate shared memory
	segmentId = shmget(mem_key, sizeof(struct times), 0666);
	if(segmentId < 0){
		printf("\nError in shmget\n");
		exit(1);
	}
		
	//attach shared memory	
	timesPtr = (struct times*) shmat(segmentId,NULL,0);
	if((intptr_t)timesPtr == -1){		
		printf("Error in shmat");
		exit(1);
	}

	char str[20];
	strcpy(str, argv[0]);
	int dur = atoi(str);
	
	int time = dur + timesPtr->nanoseconds;
		
	
	while(1){
		if(timesPtr->nanoseconds > time){
			printf("\nChild PID: %d\n",getpid());
			printf("Current Clock Value:\n");
			printf("Nanoseconds: %u\n", timesPtr->nanoseconds);
			printf("Seconds: %d\n", timesPtr->seconds);
			printf("This processes will now terminate\n\n");
			break;
		}
	}

	//save current clock
	//add duration to that value
	
	shmdt((void *) timesPtr);
	return 0;
}
