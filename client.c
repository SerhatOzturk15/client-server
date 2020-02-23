#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/msg.h>

//hold parameters in structure
struct node{
	char string[100];
	struct node *next;
};

int main(int argc, char *argv[]){
	mqd_t repq;
	mqd_t reqq;
	//get pid and set it to char
	int clientPid = getpid();
	char mq_name[128];
	sprintf(mq_name, "%d", clientPid);
	//adding slash to queue name
	char * s = NULL;
	asprintf(&s, "%s %s", "/", mq_name);
	strncpy(mq_name, s, sizeof(mq_name));
	//create the reply queue
	repq = mq_open(mq_name, O_CREAT, 0666, NULL);
	if (repq ==-1) {
	    printf("first");
	perror("can not open msg queue\n");
	exit(1);
	}
	//initialize req queue name and open it
	char reqName[100];
	strncpy(reqName, argv[1], sizeof(reqName));
	//adding slash to req queue name

	//open request to send messages to server
	reqq =mq_open(reqName, O_WRONLY|O_NONBLOCK, 0666, NULL);
	//printf("RQ created, mq id = %d\n", (int) reqq);
	if (reqq ==-1) {
	    printf("second");
	perror("can not open msg queue\n");
	exit(1);
	}
	char deneme[128];
	mq_send(reqq, (char*)mq_name,sizeof(deneme), 0);

	int j = 0;
	int N = atoi(argv[3]);
	int inputs = 2; 
	for(j; j<N+2;j++){
		char * s3 = NULL;
		asprintf(&s3, "%s" , argv[inputs]);
		strncpy(deneme, s3, sizeof(reqName));
		mq_send(reqq, (char*)deneme,sizeof(deneme), 0);
		//printf("%s", deneme);
		inputs++;
	}
	//send char for ending client send
	mq_send(reqq, (char*)"end",sizeof(deneme), 0);
	
	
	// print received data from server one by one
	char *bufptr;
	int buflen = 100000;
	bufptr = (char *) malloc(buflen);
	int pageTotal = N;
	char keyword[100];
	strncpy(keyword, argv[2], sizeof(keyword));
	printf("keyword: %s\n", keyword);
	int k = 0;
	for(k;k<pageTotal;k++){
		int n = mq_receive(repq, bufptr,buflen,NULL); //filename
		if (n == -1) {
			perror("mq_receive failed\n");
		}
			printf("<%s>", bufptr);
			n= mq_receive(repq, bufptr,buflen,NULL);
			printf(" [%s]:", bufptr);
			int l = atoi(bufptr);
			//get numbers one by one
			int sth=0;
			for(sth;sth<l;sth++){
				n= mq_receive(repq, bufptr,buflen,NULL);
				printf(" %s", bufptr);
			}
		printf("\n");
	}
	mq_unlink(mq_name);
    exit(0);
	
}
