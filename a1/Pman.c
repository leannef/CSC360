#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <sys/wait.h>   /* Wait for Process Termination */
#include <stdlib.h>     /* General Utilities */
#include <stdbool.h>
/*
     CSC360 Assignment1
	Leanne Feng
	V00825004
*/
/*
Object: struct proc
Use: Information about a single process
Attributes:
    pit_t pid - process id
    char *cmd - command and arguments  
*/

#define MAXPROC 1234
#define MAX_CMDARGS 12
typedef struct {
    	pid_t pid;
    	char *cmd;
} proc;
proc *proc_list[MAXPROC];
int count=0;
void update_bg_process() {
	int status;
	int i;
	for(i = 0; i < count; i++){
		while(1) {						//use while loop to monitor all process with -1 option.
    			int id = waitpid(proc_list[i]->pid, &status, WNOHANG);  //to monitor the state of all child process WNOHANG:return immediately if no child has exited	
			if (id == 0){					//none of children changed state, no proc_list update needed. break while loop.
				break;
			}else if (id > 0){				//Inform user that process [id] terminated. remove the item where pid = id in proc_list				
				printf("Process id %ld terminated.\n", (long)proc_list[i]->pid);
				count--;       					
				proc_list[i] = proc_list[count];	//swap the last process
          				
			}else if(id < 0){
				break;		
			}	
		} 				
	}  
}
int read_pstat_stat(char* mypid){
	char path[9999];
	size_t len = 0;
  	char* line = NULL;
  	char* token;
  	const int statLen = 20;
  	int i = 0;
  	sprintf(path, "/proc/%s/stat" , mypid );
  	FILE *fp = fopen(path, "r") ;
  	if (fp == NULL){
    		return -1;
  	}if( getline(&line,&len,fp) == -1){
      		printf("Failed to read proc file\n");
      		fclose(fp);
     		return -1;
    	}
  	token = strtok(line, " ");	//use space to separate /proc/pid/stat content
  	while(token != NULL){
      		switch(i){
        		case 1:
          			printf("comm: %s\n", token);
          			break;
        		case 2:
          			printf("state: %s\n", token);
         			break;
        		case 13:
          			printf("utime: %s\n", token);
          			break;
       	 		case 14:
          			printf("stime: %s\n", token);
          			break;
        		case 23:
          			printf("rss: %s\n", token);
         	 		break;
        	}

      		token = strtok(NULL, " ");
     	 	i++;
    	}
 	fclose(fp);
	free(line);
 	return 0;
}

int read_pstat_status(char* mypid){
	char path[9999];
  	size_t len = 0;
  	char* line = NULL;
  	char* token;
  	sprintf(path, "/proc/%s/status" , mypid );
  	FILE *fp = fopen(path, "r") ;
 	if (fp == NULL){
    		return -1;
  	}
  	while( getline(&line, &len, fp) != -1){
      		token = strtok(line, " :");	//use space to separate /proc/pid/status content
     		if(token != NULL){
          		char* str1 = "voluntary_ctxt_switches";
          		char* str2 = "nonvoluntary_ctxt_switches";
          	if(strcmp(token, str1) == 0){
              		token = strtok(NULL, " :\n");
             		printf("%s: %s\n", str1, token); 	 //print arguments after voluntary_ctxt_switches :
            	}if(strcmp(token, str2) == 0){
              		token = strtok(NULL, " :\n");	
             		printf("%s: %s\n", str2, token);	//print arguments after nonvoluntary_ctxt_switches :
            	}
        }
    	} 
	fclose(fp);
  	free(line);
	return 0;
}

int main(){

	char *input ;
    	char *prompt = "PMan:>";  
	//input = readline(prompt); 
	//printf("hahah");      	
      	int i ;
	int p;

	while( input = readline(prompt) ) {
		update_bg_process();	                              
		char* cmdArgs[MAX_CMDARGS];	//the arguments separated by space
      		char* token;			
		i = 0;
		token = strtok(input, " ");
		while(token != NULL){
			cmdArgs[i] = token;
          		token=strtok(NULL," ");	//use " " to separate string
        		i++;
        		if(i >= MAX_CMDARGS){
              			printf("Command line too many arguments!\n");
              			break;
            		}
        	}
		//printf("past strtok\n");
        if(strcmp(input, "bgkill")==0 ){	//input = bgkill 
		//kill(child_pid, SIGTERM);
        	//handle signal using kill() ;
          	if(i != 2){
              		printf("pid needed after bgkill command\n");
			continue;
		}else{
              		printf("Process id %s has teminated the job.\n" , cmdArgs[1]);
			kill(strtol(cmdArgs[1], NULL ,0), SIGTERM);		
            	}
		//}	
        }else if (strcmp(input, "bgstart")==0 ){	//input = bgstart 
        	//handle signal using kill() ;
          	if(i != 2){				//if its not exactly 2 arguments	
              		printf("Need pid after bgstart\n");
			continue;
            	}else{
              		printf("Proces id %s has restarted the job.\n" , cmdArgs[1]);
              		kill(strtol(cmdArgs[1], NULL ,0), SIGCONT);	
            	}			
        }else if (strcmp(input, "bgstop")==0 ){		//input = bgstop
          	if(i != 2){				//if its not exactly 2 arguments	
              		printf("Need pid after bgstop\n");
			continue;
            	}else{
              		printf("Proces id %s has stopped the job.\n" , cmdArgs[1]);
              		kill(strtol(cmdArgs[1], NULL ,0), SIGSTOP);		
            	}			
        }else if (strcmp(input, "bg")==0) {	//input  = bg cmd
		if(i ==1){			//if only typed with bg
              		printf("Need command after bg\n");
            	}else{
              		pid_t pid= fork();			//1. pid = fork();		
            		if (pid == 0){  			//fork() returns 0 to the child process 
            			//printf("IN CHILD PROCESS\n");	//2. in child proces :  exec*(cmd)
				if (execvp(cmdArgs[1],cmdArgs) < 0) {		//-1 result in execute error
         				printf("Invalid bg command\n");	
					//continue;	
        			}
    			}else if( pid > 0){						//parent
				proc_list[count] = malloc(sizeof(proc));		//3. create a new item in proc_list using malloc().	
				proc_list[count]->cmd =cmdArgs[1];			//4. store pid, cmd,  in new item.
				proc_list[count]->pid = pid;				//5. add new item in proc_list.
      				count++;									
    			}else{								//pid < 0
				printf("Error fork() process\n");
				continue;
    			}  			
		}			      
        } else if (strcmp(input, "pstat")==0) { 	//input = pstat pid
		if(i != 2){				//if its not exactly 2 arguments				
              		printf("Need pid after pstat\n");
			continue;
            	}else{  		
			//pid_t statid = strtol(cmdArgs[1], NULL, 0);
			printf("Printing stats for process with PID %ld \n",strtol(cmdArgs[1], NULL, 0));	//list information of pid.
			if(read_pstat_stat(cmdArgs[1]) >= 0 && read_pstat_status(cmdArgs[1]) >= 0 ){								
				continue;
    			}else{
      				printf("Error: Process %ld does not exist.\n",strtol(cmdArgs[1], NULL, 0));
      				continue;
    			}	
		}

        } else if( strcmp(input, "bglist")==0 ) {
		update_bg_process();
		if(i > 1){						//if more than 1 argument
			printf("Invalid command. Only need bglist"); 
			continue;
		}
		int i;
		for(i = 0; i < count; i++){
      			printf("%ld:  %s\n", (long)proc_list[i]->pid, proc_list[i]->cmd);	//pirnt current processes
    		}
  			printf("Total background jobs: %d\n" , count);
        } else {
        	printf("%s: command not found\n", input);	//type in nothing or error format will result in command not found
		continue;
        }
				
	}
   
	return ;

}


