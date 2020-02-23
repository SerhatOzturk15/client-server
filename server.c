#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/msg.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>

//THIS IS SERVER SIDE OF MY CLIENT SERVER APPLICATION HANDLES COMMUNICATION BETWEEN CLIENT AND SERVER.

//this is to hold incoming message words in a linkedlist
struct node{
	char string[100];
	int number; //for file description to pass to the threads
	int wordCounter; //returns number of matches
	int lineCounter[100]; //returns line numbers
	char t_keyword[128];
	char *outputString[1000];
	struct node *next;
};

static void *dotheJob(void *argPtr){
	//initialize thread spesific variables
	char fileNames[1000];  //file names to search file
	char searchWord[1000];  //the keyword to search
	char lines[1000];   // store each line in it
	strncpy(fileNames, ((struct node *) argPtr)->string, 128);
	int fileID= ((struct node *) argPtr)->number;
	strncpy(searchWord, ((struct node *) argPtr)->t_keyword, 128);

	//open files and read them.
	FILE *theFile;
	theFile = fopen(fileNames, "r");
	if(theFile == 0){
		printf("could not open the file\n");
		}
	((struct node *) argPtr)->wordCounter = 0; //first set counter 0
	int i = 0; //to get lines for multiple occurence
	char *cuttedWord;
	char *cuttedWord2;
	int matchedLines = 1;

	while(fgets(lines, sizeof(lines), theFile)){
		//char * s10 = NULL;
		//asprintf(&s10, "%s%s", lines, " klm");
		//strncpy(lines, s10, sizeof(lines));
		
		//get first word
			char *saveptr;
		cuttedWord = strtok_r(lines, " ", &saveptr);
		if(strcmp(cuttedWord,searchWord ) == 0){
			((struct node *) argPtr)->wordCounter++;
			//printf("counter is:%d\n",((struct node *) argPtr)->wordCounter);
			((struct node *) argPtr)->lineCounter[i] = matchedLines;
			i++;
		}
	
		//rest
		while((cuttedWord2 = strtok_r(NULL, " \n", &saveptr)) != NULL){
				//printf("SIZE is:%d\n",sizeof(cuttedWord));
				//printf("words outside strcmp:-%s-\n", cuttedWord);
			if(strcmp(cuttedWord2,searchWord ) == 0){
				((struct node *) argPtr)->wordCounter++;
				//printf("words inside strcmp: %s\n", cuttedWord);
				//printf("counter is:%d\n",((struct node *) argPtr)->wordCounter);
				((struct node *) argPtr)->lineCounter[i] = matchedLines;
				i++;
				}//endif
			}//endwhile
		matchedLines++; //keep which lines are matched
	}//end-mainwhile
	//printf("number of mathes are: %d \n",((struct node *) argPtr)->wordCounter );
	
	pthread_exit(NULL);
}

int main(int argc, char *argv[]){
		mqd_t reqq;
	mqd_t repq;
	char reqName[100];
	strncpy(reqName, argv[1], sizeof(reqName));
	//adding slash to req queue name
	//printf("after op %s\n", reqName);
	//char * s2 = NULL;
	//asprintf(&s2, "%s %s", "/", reqName);
	//strncpy(reqName, s2, sizeof(reqName));
	//printf("req queue name is: %s\n", reqName);
	//open the request queue for read and creation
	reqq = mq_open(reqName, O_RDWR|O_CREAT, 0666, NULL);
	//printf("RQ created, mq id = %d\n", (int) reqq);
	if (reqq ==-1) {
	    printf("üçüncü");
	perror("can not open msg queue\n");
	exit(1);
	}

	bool child = false;
	int buflen = 100000;
	char *bufptr;
	bufptr = (char *) malloc(buflen);
	char dene[128];
	int running = 1;
	int i=0;
	int N = 0;
	pid_t id[N];
    int counter = 0;
	struct node* head;
	struct node* follow;
	while(running){
		int n = mq_receive(reqq, bufptr,buflen,NULL);
		if (n == -1) {
			perror("mq_receive failed\n");
			//exit(1);
		}
		//if the message received from queue, create a child process to do it.
		else{
			if(counter == 0){
			    head = malloc(sizeof(struct node));
			    follow = head;
				strncpy(head->string, bufptr, 100);
				counter++;
			}
			else{
				follow->next = malloc(sizeof(struct node));
				strncpy(follow->next->string, bufptr, 100);
				follow = follow->next;
				}
				if(strcmp(bufptr, "end") == 0){
				    counter = 0;
					id[N] = fork();
					if(id[N] < 0){
						printf("could not create the child");
						exit(-2);
					}
					if(id[N] == 0){
						running = 0;
						child = true;		
					}
				N = N+1;
				
			}	
		}	
	}
	
	if(child = true){
		//lets set request queue parameters to usable attributes here
		char replyQ[100];
		char keyword[100];
		char numOfFiles[100];
		struct node* usable = head;
		strncpy(replyQ, usable->string, 100);
		usable = usable->next;
		strncpy(keyword, usable->string, 100);
		usable = usable->next;
		strncpy(numOfFiles, usable->string, 100);
		usable = usable->next;
		int num = atoi(numOfFiles);
		int count = 0;
		int ret;
		pthread_t tids[num];
		int k;
		struct node t_args[num];
		struct node* filesBeginning = usable;
		struct node* filesBeginning2 = usable;
		
		//arranging what will pass to the threads
		for(count; count<num; count++){
			strncpy(usable->t_keyword, keyword, 100);
			 usable->number=count; 
			usable= usable->next;	
		}
		int threadCounter = 0;
		for(threadCounter; threadCounter<num; threadCounter++){
			
			ret = pthread_create(&tids[threadCounter],NULL, dotheJob, (void*) filesBeginning);
			if(ret !=0){
				printf("thread creation failed \n");
				exit(2);
			}
			filesBeginning = filesBeginning->next;
		}
		
		// joining threads and handling error
		for(k =0; k<num;k++){
			ret = pthread_join(tids[k], NULL);
			if(ret != 0){
				fprintf(stderr, "Error joining thread\n");
				exit(0);
			}
		}
		
		//open reply queue
		int f = 0;
		char deneme[128];
		repq = mq_open(replyQ, O_RDWR, 0666, NULL);
		if (repq ==-1) {
		    printf("dördüncü");
			perror("can not open msg queue\n");
			exit(1);
		}

		//send messages
		char wordCounterChar[128];
		struct node* filesBeginning3 = filesBeginning2;
		//convert line numbers to char array to pass to mq_send
					
			
		for(f; f<num; f++){
			//mq_send(repq, (char*),sizeof(replyQ), 0);
			mq_send(repq, (char*)filesBeginning2->string,sizeof(replyQ), 0);
			//convert word counter to char
			sprintf(wordCounterChar, "%d",filesBeginning2->wordCounter);
			mq_send(repq, (char*)wordCounterChar,sizeof(replyQ), 0);
			int iter =0;
			char charLineNumbers[128];
			for(iter;iter<filesBeginning2->wordCounter;iter++){
				sprintf(charLineNumbers, "%d",filesBeginning2->lineCounter[iter]);
				mq_send(repq, (char*)charLineNumbers,sizeof(replyQ), 0);
			}
									
			filesBeginning2 = filesBeginning2->next;	
		}		
		//send messages
		exit(0);
		
	} //child exit
	
	mq_close(repq);
	
	
}
