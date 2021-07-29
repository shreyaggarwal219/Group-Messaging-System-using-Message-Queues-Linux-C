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
#define SERV_PORT 3000

//client for the group messaging system

//structure for the message to be sent from different client to server using the message queue

int requestingGroupList=0;
int deleteTime =-1;
pthread_mutex_t lock;
int sendingMsg=1; 
char username[20];
struct message{
    long mtype;
    time_t time;
    char sentfrom[20];
    char sendto[20];
    char groupName[20];
    char msg[100];
    char groupList[100][20];
    int groupNo;
    int deletetime;
};

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

//function for receiving the messages from the server
void * output(void *arg)

{  
    struct message myMsg;
    int my_msgid;
    int hash = compute_hash(username);
    if((my_msgid = msgget((key_t)hash,IPC_CREAT|0644))==-1)
    {
   perror("msgget");
   exit(1);
    }
    printf("My msgid is : %d \n",my_msgid);
    while(true)
    {
     //   printf("Thread two started \n");
    pthread_mutex_lock(&lock);
   // printf("Inside Lock \n");
    if(msgrcv(my_msgid,&myMsg,sizeof(myMsg),0,IPC_NOWAIT)==-1)
    {  
         //perror("msgrcv");
        pthread_mutex_unlock(&lock);
         sleep(5);
         continue;
    }
   // printf("Recieved Message \n");
        time_t now = time(0);
        if((now-myMsg.time >= deleteTime)&&deleteTime>=0)
        {
           continue;
        }   
        switch (myMsg.mtype)
        {
        case 1 :   
            printf("********************************************\n");
            printf("You have recieved a new message \n");
            printf("%s : %s \n",myMsg.sentfrom,myMsg.msg);
            printf("********************************************\n");
            break;
        case 2 : 
            printf("********************************************\n");
            printf("You have recieved a new message on the group %s \n",myMsg.groupName);
            printf("%s : %s \n",myMsg.sentfrom,myMsg.msg);
            printf("********************************************\n");
            break;
        case 3 : 
            printf("********************************************\n");
            printf("Following Groups Exist \n");
            for(int i=1;i<=myMsg.groupNo ;i++)
            printf("%s \n",myMsg.groupList[i]);
            printf("********************************************\n");
            requestingGroupList=1;
            break;
        default:
            break;
        }

 


    pthread_mutex_unlock(&lock);
    }
    msgctl(my_msgid,IPC_RMID,0);
}

//function for printing the menu for the group messaging system
void printMenu()
{   
    printf("\e[1;1H\e[2J");
    printf("\n################################\n");
    printf("Press 1 to send Private Message \n");
    printf("Press 2 to send message to a Group \n");
    printf("Press 3 to list existing Groups \n");
    printf("Press 4 to create group \n" );
    printf("Press 5 to Join group \n");
    printf("Press 6 to set delete time \n");
    printf("################################\n \n");
}





int main(int argc, char **argv)
{
int choice;
int running = 1;
int msgid;
struct message msg;
long int msg_to_recieve = 0;
msgid = msgget((key_t)1234, 0666 | IPC_CREAT);

if (msgid == -1) {
	  perror("msgget failed with error");
	  exit(EXIT_FAILURE);
	}
else
printf("MQ created with msgid %d \n", msgid);

if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }

pthread_t thread_id;
printf("Before Thread\n");




printf("Enter your user name: \n");
scanf("%s",username);
pthread_create(&thread_id, NULL,output, NULL);
while(true)
{
pthread_mutex_unlock(&lock);
printMenu();
scanf("%d",&choice);
//printf("Choice taken \n");
pthread_mutex_lock(&lock);
//printf("Inside main lock \n");
msg.time = time(0);
switch (choice)
{
case 1: printf("Enter the name of the user you wish to send message \n");            //private message request
    scanf("%s",msg.sendto);
    printf("Enter the message: \n");
    while ((getchar()) != '\n');
    scanf("%[^\n]",msg.msg);
    strcpy(msg.sentfrom,username);
    msg.mtype=1;
    printf("Sending Message: %s  with type %ld \n",msg.msg,msg.mtype);
    if(msgsnd(msgid,&msg,sizeof(msg),0)==-1)
    // perror("Messang was not sent");
    // printf("Message was sent \n");

 //   printf("%lld \n",compute_hash("Shivam"));
    
    pthread_mutex_unlock(&lock);
    //printf("Main lock released");
    break;

case 2: printf("Enter the name of the group \n");                                  //group message request
        scanf("%s",msg.groupName);
        printf("Enter the message: \n");
        while ((getchar()) != '\n');
        scanf("%[^\n]",msg.msg);
        strcpy(msg.sentfrom,username);
        msg.mtype=2;
        printf("Sending Message: %s  with type %ld \n",msg.msg,msg.mtype);
        if(msgsnd(msgid,&msg,sizeof(msg),0)==-1)
        perror("Messang was not sent");
        pthread_mutex_unlock(&lock);
        break;
case 3: printf("Requesting Group List \n");                                         //list group request
        msg.mtype = 3;
        strcpy(msg.sentfrom,username);
        if(msgsnd(msgid,&msg,sizeof(msg),0)==-1)
        perror("Messang was not sent");
        pthread_mutex_unlock(&lock);
        while(requestingGroupList==0)
        {sleep(1);}
        requestingGroupList=0;
        break;
case 4: printf("Enter the name of the group you want to create \n");               //ctreate group request
        scanf("%s",msg.groupName);
        msg.mtype = 4;
        strcpy(msg.sentfrom,username);
        if(msgsnd(msgid,&msg,sizeof(msg),0)==-1)
        perror("Messang was not sent");
        pthread_mutex_unlock(&lock);
        break;
case 5: printf("Enter the name of the group you want to join \n");                //joining a group request
        scanf("%s",msg.groupName);
        msg.mtype = 5;
        strcpy(msg.sentfrom,username);
        if(msgsnd(msgid,&msg,sizeof(msg),0)==-1)
        perror("Messang was not sent");
        pthread_mutex_unlock(&lock);
        break;
case 6: printf("Enter the name of the group \n");                                 //changing the auto delete time of the group request
        scanf("%s",msg.groupName);
        msg.mtype = 6;
        strcpy(msg.sentfrom,username);
        printf("Enter the delete time \n");
        scanf("%d",&msg.deletetime);
        if(msgsnd(msgid,&msg,sizeof(msg),0)==-1)
        perror("Messang was not sent");
        pthread_mutex_unlock(&lock);
        break;
default: 
    printf("Invalid Choice \n");
    break;
}
}
pthread_join(thread_id, NULL);
}
