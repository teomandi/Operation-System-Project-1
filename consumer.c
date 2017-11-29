#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>


#define in_dsSIZE  1024
#define out_dsSIZE 1024	


void consumer(char* shmemIN, char* shmemOUT){
	char line[200];
    char backup[1024], backup2[1024];
    strcpy(backup, shmemIN); 
    char s[2]="\n", *token;
    token = strtok(backup, s);
    strcpy(line, token);					     //we keep first line for CAPITALIZE
    int i=1;
    while( token != NULL ) {
       	token = strtok(NULL, s);
       	if (token == NULL)
       		break;
       	if(i==1){							        //fix the rest to the proper form
       		strcpy(backup2,token);
       		strcat(backup2,"\n");
       		i=0;
       	}
       	else{
       		strcat(backup2,token);
       		strcat(backup2,"\n");
       	}
    }
    Cap(line);								        //capitalize
    strcat(line,"\n");
    if(strlen(backup2)>in_dsSIZE){
    	printf("ERROR: in_ds is full..\n");
    	return;
    }
    strcpy(shmemIN,backup2);
    if((strlen(shmemOUT)+strlen(line)) > out_dsSIZE){
		printf("ERROR:out_ds is full..\n");
		return;
	}
    if(strlen(shmemOUT)== 0)
    	strcpy(shmemOUT, line);
    else
    	strcat(shmemOUT,line);
}


void consumerP2(char *shmemOUT){
	strcpy(shmemOUT,"end");					     //end messege has been put
	printf("out:%s!!\n",shmemOUT );
}


void Cap(char string[]){    				   //takes a String and Capitalizes it 
    int i;
    int x = strlen(string); 
    for (i=1;i<x;i++){
        if (isalpha(string[i])){ 
            string[i]= toupper(string[i]);
        }
    }
}