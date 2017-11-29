#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <signal.h>

#include "umers.h"

#define in_dsSIZE  1024
#define in_dsKEY (key_t) 2341 
#define out_dsSIZE 1024
#define out_dsKEY (key_t) 3412
#define pmSIZE 8
#define pmKEY (key_t) 3413


#define SEMKEY 1234

union semun {
	int val;
	struct semid_ds *buff;
	unsigned short *array;
};



struct sembuf oparray;

void declaration_op(int x,int y){           //function to make the changes to oparray easily
    oparray.sem_flg=0;
    oparray.sem_op=y;
    oparray.sem_num=x;
}


int main(int argc, char *argv[]){
	int count, C, N, semid, inid, outid, pmid, i, pid, cid;
	char* textname;

//********************************************ARGUMENTS CONTROL************************************************\/
	printf ("This program was called with \"%s\".\n",argv[0]);
  	if (argc > 1){
    	for (count = 1; count < argc; count++){
    		printf("argv[%d] = %s\n",count, argv[count] );
    		if(strcmp(argv[count], "-i")==0){	//-i:input
    			textname = (char *) malloc(sizeof(strlen(argv[count+1])));
	  			strcpy(textname,argv[count+1]);
        	}
	  		else if(strcmp(argv[count], "-n")==0)
	  			sscanf(argv[count+1], "%d", &N);
	  		else if(strcmp(argv[count], "-c")==0)
	  			sscanf(argv[count+1], "%d", &C);
    	}
    }
	else
    	printf("The command had no other arguments.\n");
//*************************************************************************************************************\/
   	inid = shmget(in_dsKEY, in_dsSIZE, IPC_CREAT | 0600);       //initializations
   	outid = shmget(out_dsKEY, out_dsSIZE, IPC_CREAT | 0600);
    pmid = shmget(pmKEY, pmSIZE, IPC_CREAT | 0600);
    char *shmemPM, *shmemIN, *shmemOUT;
    
    if((shmemPM = shmat(pmid,(char *) 0 , 0))==(char*) -1){     //atouches
        printf("ERROR: error in ato pidmatch.\n");
        exit(1);
    }
    strcpy(shmemPM,"0");//init pid_match with zero

    if((shmemIN  = shmat(inid,(char *) 0 , 0)) == (char*)-1){
        printf("Producer: ERROR: failed to atouch IN_DS.\n");
        return;
    }
    if((shmemOUT = shmat(outid,(char *) 0 , 0)) == (char*)-1){
        printf("Consumer: ERROR: failed to atouch OUT_DS.\n");
        return;
    }
    union semun arg;
	semid =semget(SEMKEY,N+1, IPC_CREAT | 0600);   //creating N+1 semaphores to control the flow
	arg.val=0;	
	semctl(semid, 0, SETVAL, arg);
    printf("*START*\n");
    srand(time(NULL));                              //random generator
    if((cid = fork())<0){                           //consumer creation
        printf("ERROR : error in fork.\n");
        exit(1);
    }
    if(cid == 0){
        int k, q=1;
        for(k=0;k<C;k++){
            declaration_op(0,-1);
            semop(semid, &oparray,1);
            consumer(shmemIN, shmemOUT);
            if(q==N)                            //which process should wake up
                q=1;
            else
                q++;
            declaration_op(q,1);
            semop(semid, &oparray,1);
        }
        consumerP2(shmemOUT);                   //consumer informs the other processes to stop via out_ds
        printf("*K repeats complited.\n*Messega to end programm sent.\n");
    }
    else{
        for(i=1;i<=N;i++){
    	    if((pid = fork())<0){              //creating N producers
                printf("ERROR : error in fork.\n");
                exit(1);
            }
        	if (pid == 0){
                while(1){
                    declaration_op(i,-1);       //all the processes geting down
                    semop(semid, &oparray,1);
                    
                    int r = rand();
                    producerP1(textname, shmemIN, r, i);
                    declaration_op(0,1);         //wake up consumer     
                    semop(semid, &oparray,1);
                    declaration_op(i,-1);
                    semop(semid, &oparray,1); 
                    int flag = producerP2(shmemOUT, shmemPM);   //when waked up do his job.... if returns 1 then kill process
                    if(i==N){
                        declaration_op(1,1);            
                        semop(semid, &oparray,1); 
                    }
                    else{
                        declaration_op(i+1,1);            
                        semop(semid, &oparray,1);
                    }
                    if (flag == 1){
                        exit(1);
                    }
                }
                break;
        	}
        	else
                continue;
        }
    }
    if(cid!=0 ){
        declaration_op(1,1);                //triger: wake up the first producer
        semop(semid, &oparray,1);
    }
    wait();
    for(i=0;i<N;i++){
        wait();
    }
    if(cid > 0){
        int pm = atoi(shmemPM);
        printf("*****RESULTS***** %d \npid_match: %d\nC = %d, N = %d \n",getpid(), pm,C,N);
        printf("****END****\n");
    }
    free (textname);                        //sum ups
    shmdt(shmemPM);
    shmdt(shmemOUT);
    shmdt(shmemIN);
    shmctl(pmid, IPC_RMID,  0 );
    semctl(semid, 0, IPC_RMID,0); 
    shmctl(inid, IPC_RMID,  0 );
    shmctl(outid, IPC_RMID,  0 );
    shmctl(pmid, IPC_RMID,  0 );    
}