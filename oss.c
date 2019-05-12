#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "clock.h"


void exitProgram(pid_t pids[], int n);

//Used to exit program by ctrl+c and the 2 seconds timer
int exitFlag = 0;
void ossExit(int sig){
	switch(sig){
		case SIGALRM: 
			printf("\nTwo seconds is up!\n");
			break;
		case SIGINT: 
			printf("\nUser has exited using ctrl+c\n");
			break; 
	}
		
	exitFlag = 1;
}





int main(int argc, char *argv[]){
	signal(SIGALRM, ossExit);
	alarm(2);

	signal(SIGINT, ossExit);
	
	int n = 0;//Store max number of children
	int s = 0;//Max number in system at any given time
	char *i = NULL; //Inputfile
	char *o = NULL;//Outputfile
	//Get command line arguments	
	int c;	
	while((c=getopt(argc, argv, "n:s:i:o:h"))!= EOF){
		switch(c){
			case 'h':
				printf("\n\n-n: Total number of processes(Default of 4)");
				printf("\n-s: Total number of processes in machine at any given time(Default of 2)");
				printf("\n-i: Specify default inputfile(Default is input.txt)");
				printf("\n-o: Specify default outputfile(Default is output.txt)\n\n");
				exit(0);
				break;
			case 'n':
				n = atoi(optarg);
				break;
			case 's':
				s = atoi(optarg);
				if(s > 20){
					printf("Maximum number of child processes running on machine is 20. Setting s to 20.");
					s = 20;
				}
				break;
			case 'i':
				i = optarg;
				break;
			case 'o':
				o = optarg;
				break;
		}
	}
	//set defailt files
	if(i == NULL){
		i = "input.txt";
		
	}
	if(o == NULL){
		o = "output.txt";
	}
	if(n == 0){
		n = 4;
	}
	if(s == 0){
		s = 2;
	}

	//open inputfile
	FILE *fp;
	fp = fopen(i,"r");
	if(fp == NULL){
		fprintf(stderr, "%s: Error in %s: ", argv[0] , i);
		perror("");
		exit(0);
	}

	//open outputfile
	FILE *ofp;
	ofp = fopen(o,"w");
	if(ofp == NULL){
		fprintf(stderr, "%s: Error in %s: ", argv[0], o);
		perror("");
		exit(0);
	}
	
	//create clock
	createClock();
		
	//line buffer to store entire of line of inputfile
	char line[100];
	int time1, time2,incrementor;//Integers to store from inputfil
	char duration[20];
	
	//Set the clock incrementor
	if(fgets(line, 100, fp)!= NULL){
		incrementor = atoi(line);
	}

	//Array to store all child proceses that occur
	pid_t pids[n], child;
	int k = 0;
	int p;
	//Holds number of active child processes
	int active, terminated, total = 0;
	int status;	
	int max = n;
	int read = 1;
	
	//Start loop
	while(n != 0 || terminated != max){
		//increment clock	
		incrementClock(incrementor);
	
		if(exitFlag == 1){
			exitProgram(pids,max);
		}
		//launch child--pass the duration(how long child to stay in system)
		if(n !=0 && active < s){	
			//Grab the two times and duration from file
			if(read == 1){
				fscanf(fp,"%d %d %s\n", &time1, &time2, &duration);
				read = 0;
			}

			if(getNanoseconds() >= time1 && getNanoseconds() >= time2){
				
				read = 1;
				k++;
				active++;
				n--;
				//fork and exec
				pids[k] = fork();			
				if(pids[k]  == 0){
					//printf("A fork has executed\n");
					execl("./user", duration, NULL);
					exit(0);	
				}
			}		
		}
		
		

		//check if one child ended
		child = waitpid(-1, &status, WNOHANG);
		if(child > 0){
			
			fprintf(ofp, "Child Process with pid: %d Terminated in %d Seconds, or %u Nanoseconds.\n", child, getSeconds(), getNanoseconds());
			terminated++;
			active--;
		}
	}
	
	fprintf(ofp, "Program ended with clock at %d seconds or %u nanoseconds.\n", getSeconds(), getNanoseconds());
	
	unallocateClock();

	fclose(fp);
	fclose(ofp);
	return 0;


}

void exitProgram(pid_t pids[], int n){
	int k;
	unallocateClock();
	for(k = 0; k < n; k++){
		kill(pids[k],SIGKILL);
	}
	exit(0);
}


