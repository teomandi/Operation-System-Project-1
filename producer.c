#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>

#define in_dsSIZE  1024
#define out_dsSIZE 1024
#define pmSIZE 8


void producerP1(char * textname, char* shmemIN, int r, int i){
	char text[500];
	char strpid[10];
	int pid = getpid();
	sprintf(strpid, "%d", pid);
	FILE * fp;
	fp = fopen(textname, "r");								//open file
	if (fp == NULL)
		printf("Failed to open file\n");
	char * line = NULL;  
	size_t len = 0;
	ssize_t read;	
	while((read = getline(&line, &len, fp))!= -1){			//read lines until to get "random" line
		if(r%i==0){
			printf("taken: %s\n",line);
			break;
		}
		else
			continue;
	}
	if(strlen(line)==0){
		printf("ERROR IN LINE:empty line\n");				//just in case
		return;
	}
	strcpy(text,strpid);
	strcat(text,"|");
	strcat(text, line);
	strcat(text, "\n");										//creating to the appear like <pid_num>|<line>\n
	if((strlen(shmemIN)+strlen(text)) > in_dsSIZE){			
		printf("ERROR: in_ds is full..\n");
		return;
	}
	if(strlen(shmemIN)==0)
		strcpy(shmemIN,text);
	else
		strcat(shmemIN, text);
	fclose(fp);
}



int producerP2(char* shmemOUT, char* shmemPM){
	int k,pid = getpid();
	char backup[1024], backup2[1024];
	printf("out:%s\n",shmemOUT );
	if(strcmp(shmemOUT,"end")==0)							//if its end return 1 to kill the process
		return 1;
	if(strlen(shmemOUT) == 0){
		printf("Error: OUT_DS is empty.");
		return 0;
	}
	strcpy(backup, shmemOUT);
	char line[200];
	char s[2]="\n", *token;
    token = strtok(backup, s);
    strcpy(line, token);
    int i=1;
    while( token != NULL ) {								//take the first line to make the check and then fix the rest
       	token = strtok(NULL, s);
       	if (token == NULL)
       		break;
       	if(i==1){
       		strcpy(backup2,token);
       		strcat(backup2,"\n");
       		i=0;
       	}
       	else{
       		strcat(backup2,token);
       		strcat(backup2,"\n");
       	}
    }
    char num[6], seira[200];
    int flag=1, d;
    for(k=0;k<strlen(line);k++){							//takes the pid of the one who fix it and the line
    	if (isdigit(line[k]) && flag){
   			num[k]=line[k];
    	}
    	if(flag == 0) {
    		seira[k-d]=line[k];
    	}
    	if(line[k]=='|'){
    		num[k]='\0';
    		flag=0;
    		d=k+1;
    	}
    }
    strcpy(shmemOUT,backup2);						
    int pm, number = atoi(num);
   	pm = atoi(shmemPM);
   	printf("%d ==?== %d \n", pid, number);
   	if(pid == number){									//check the for match
   		printf("MATCH_FOUND\npid : %d\nline : %s\n", pid, seira);
   		pm ++;
   	}
   	else
   		printf("MATCH_NOT_FOUND\npid : %d\npid_found : %d\nline : %s\n", pid, number, seira);
   	sprintf(shmemPM, "%d", pm);
   	return 0;
}