#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>

//server for the group messaging system


//structure for the message to be sent from different client to server using the message queue 

struct mesg_buffer {                                 
   	 long m_type;
    	 time_t time;
   	 char sentfrom[20];                                     
    	 char sendto[20];
    	 char group_name[20];
    	 char mesg_text[100];
     	 char group_list[100][20];
   	 int group_no;
   	 int delete_time;
    
} message;


/*structure for the different groups created by the users(clients)
  maximum number of groups if 100
  maximum number of old messages stored is 100 for a group*/

struct group{                                            
	int participants;				
	char groupname[20];				
	char users[100][20];
	time_t users_join_time[100];
	int old_mes_num;
	struct mesg_buffer old_mes[100];
	time_t delete_time;
	

}grp[100];                                              

int grp_no =0;

/*hashing function for generating the message id of different users (assuming all users have different names)
*/
long long compute_hash(char *s) {                       
    const int p = 31;
    const int m = 1e9 + 9;
    long long hash_value = 0;
    long long p_pow = 1;
    for (int i=0;i<20;i++) {
       	char c = s[i];
      	 if(c=='\0')
      	 break;
        hash_value = (hash_value + (c - 'a' + 1) * p_pow) % m;
        p_pow = (p_pow * p) % m;
    }
    return hash_value;
}

//function for sending private messages to the clients
void private_message(struct mesg_buffer message)                                            
{

	int send_id = (key_t)compute_hash(message.sendto);    	
	send_id = msgget((key_t)send_id,0666 | IPC_CREAT);	
	if(msgsnd(send_id,&message,sizeof(message),0)==-1)
	perror("msgsnd");
}

//function for creating new groups and adding the participant 
void create_group(struct mesg_buffer message)                                               

{	
	int check_group = 1;
	for (int i =1;i<=grp_no;i++)
	{

		if(!strcmp(grp[i].groupname,message.group_name))
		check_group =0;

	}

	if(check_group)
	{
	grp_no++;
	grp[grp_no].participants = -1;
	strcpy(grp[grp_no].groupname,message.group_name);
	strcpy(grp[grp_no].users[++grp[grp_no].participants], message.sentfrom); 
	grp[grp_no].users_join_time[grp[grp_no].participants] = time(0);
	grp[grp_no].old_mes_num =-1;
	grp[grp_no].delete_time = 0;


	}

	else
	printf("A group with name %s is alredy present\n",message.group_name);
}

//function for adding new memebers of the group and sending the previous messages 
void joinGroup(char * username,char * groupName)                                         
{
	int check_user = 1, grpn;
	if(grp_no == 0)
	{	
	printf("no such group exits\n");
        check_user = 0;
	}
    	for(int i=1;i<=grp_no;i++)
    	{
        	if(strcmp(groupName,grp[i].groupname)==0)
       	 	{
        		grpn = i;
			for(int j= 0;j<=grp[i].participants;j++)
			{	    
           			if(strcmp(grp[i].users[j],username)==0)
            			{
              				printf("user already prensent in group \n");
              				check_user = 0;
              
            			}
            
        		}
        	}
        	else if(i==grp_no)
        	{
        		printf("no such group exits\n");
        		check_user = 0;
        	}
        
    	}
    	if(check_user)
    	{
   
            printf("user added\n");  
            strcpy( grp[grpn].users[++grp[grpn].participants],  username); 
            grp[grpn].users_join_time[grp[grpn].participants] = time(0);
            
            int send_id = (key_t)compute_hash(username);    	
            send_id = msgget((key_t)send_id,0666 | IPC_CREAT);
        //    printf("mq id:%d\n",send_id);
            for(int i=0;i<=grp[grpn].old_mes_num;i++)
            {
            
     //sending previous messages to the new participant of the group given that the auto delete time is more than the time between the message and their joining the group 
            	
            	if( grp[grpn].delete_time ==0 || difftime(grp[grpn].users_join_time[grp[grpn].participants],grp[grpn].old_mes[i].time)<grp[grpn].delete_time)		
            	{
                    
            		printf("sending previous message :%s\n",grp[grpn].old_mes[i].mesg_text);
           		if(msgsnd(send_id,&grp[grpn].old_mes[i],sizeof(message),0)==-1)
            		perror("msgsnd");
            	}
                               
            }
        	
   	 }

}

//function for listing the groups existing currently in the system
void listGroup()                                                                        
{
    int send_id = (key_t)compute_hash(message.sentfrom);
        struct mesg_buffer msg;
        for(int i=1;i<=grp_no;i++)
        {
            printf("%s\n",grp[i].groupname);
            strcpy(msg.group_list[i],grp[i].groupname);
        }
        msg.m_type = 3;
        msg.group_no = grp_no;
       	send_id = msgget((key_t)send_id,0666 | IPC_CREAT);
    	if(msgsnd(send_id,&msg,sizeof(msg),0)==-1)
    	perror("msgsnd");

}

//function for messaging in the groups
void group_mes(struct mesg_buffer message)                                              
{
int grpn = 0;
if(grp_no == 0)
printf("no such group exists\n");

	for(int i =1;i<=grp_no;i++)
	{
	 	if(strcmp(grp[i].groupname,message.group_name)==0)
	 	{ 
	 		grpn  = i;
	 		for(int j =0;j<=grp[i].participants;j++)
			{
	   			int send_id = (key_t)compute_hash(grp[i].users[j]);   	
         			send_id = msgget((key_t)send_id,0666 | IPC_CREAT);
    	 			if(msgsnd(send_id,&message,sizeof(message),0)==-1)
    				 perror("msgsnd"); 
    	 
    			}
		 }
	 	else if(i==grp_no)
	 	printf("no such group exists\n");
	}
	
	if(grpn)                                                                //store old messages
	{
		 message.time = time(0);
    		 grp[grpn].old_mes[++grp[grpn].old_mes_num] = message;
    	 //	 printf("%d\n",grp[grpn].old_mes_num);	
	}
   
}

//function for setting the auto delete message time of the group 
void setdeletetime(struct mesg_buffer message)                                      
{
if(grp_no == 0)
printf("no such group exists\n");

	for(int i =1;i<=grp_no;i++)
	{
	 	if(strcmp(grp[i].groupname,message.group_name)==0)
	 	{
	 		grp[i].delete_time = message.delete_time;
	 		printf("delete time set is :%d\n",message.delete_time);	 
		 }
	 	else if(i==grp_no)
	 	{
			 printf("no such group exists\n");
	 	}
	}

}

  
int main()
{   
    printf("Running Server....... \n");
    int msgid;
   
    // msgget creates a message queue

    if(msgid = msgget((key_t)1234, 0666 | IPC_CREAT)==-1)
    perror("msgget");
    
    
  //  printf("The msgid is %d \n",msgid);
    for(;;){
   
    if(msgrcv(msgid, &message, sizeof(message), 0, 0)==-1)
    {
    	perror("msgrcv");
    }
   //     printf("A message is received \n");
  //  	printf("message: %s type: %ld sentfrom %s\n",message.mesg_text,message.m_type,message.sentfrom);
    	//message.m_type =1;
    	if(fork()==0)
    	{
   //     printf("Entering switch %ld\n",message.m_type);
       
    	switch(message.m_type)
    	{
    	case 1:{//private message
    		private_message(message);
    		break;
    		
    	}
    	
    	case 2:{//group message   	
    		group_mes(message);
    		break;
    	}
    	
    	case 3:{//list group
    		listGroup();
    		break;
    	
    	}
    	case 4:{//create group
    	        
    	    //    printf("%d\n",grp_no);
    		create_group(message);
    		break;
    	}
    	case 5:{//join group
    		joinGroup(message.sentfrom,message.group_name);
    		break;
    		}
    	case 6:{//set auto delete time
    		setdeletetime(message);
    	}	
    	default: ;	
    		}
    
    }
    else 
    {
        wait(0);
    }
    }
    }
