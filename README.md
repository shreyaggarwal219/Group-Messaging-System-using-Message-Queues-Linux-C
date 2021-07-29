# Group-Messaging-System-using-Message-Queues

The group messaging system uses the message queue design using the client and server model. It
allows users in a UNIX system to create groups, list groups, join, send private messages, send
message to groups, receive message from groups in online/offline modes. A user can also set auto
delete <t> option which means, users who joined after t seconds from the time of message creation,
to them message will not be delivered.
  
SERVER DESIGN
  
• The server receives the requests from the clients and redirects the requests according to the
action performed by the client
  
• The server :
1. Sends private messages to the clients
2. Sends group messages to the clients
3. Creates new groups
4. Adds new participants to the groups
5. Sends the list of the groups to the user
6. Sets auto delete time of the message for a particular group
  
• A maximum of 100 groups can be created in the system
  
• A maximum of 100 old messages can be saved by the server for a particular group
  
• It is assumed that no two users have the same name. Only the first name of the user is
considered
  
• A hashing function is used to retrieve the message id of the user using their first name
• Different data structures used are:
  
1. struct mesg_buffer : for sending messages from server to client(message sender
name, message receiver name, type of message, message text, etc)
2. struct group : for storing the information of the groups(group name ,number of
participants, names of participants, time of joining the group of each participant,
number of old messages, old messages, auto delete time for the group )
  
• Different functions used are:
1. void private_message (struct mesg_buffer message) : takes a message from the
client and sends the message to the client specified in the sendto field of the message
structure.
2. void create_group(struct mesg_buffer message) : create a group using the
groupname specified in the message structure.
3. void joinGroup(char * username,char * groupName) : adds the user to the group
specified by the groupName and sends the previous messages in the group to the
new user. If no such group exists specified by the groupName then no new group is
created.
4. void listGroup(): lists the various groups present currently in the system.
5. void group_mes(struct mesg_buffer message): messages to every participant in the
group .
6. void setdeletetime(struct mesg_buffer message): sets the auto message delete time
for the group.
  
  
CLIENT DESIGN
  
• The client sends the request to the server according to the choice of the user:
1. Send private message
2. Send Group message
3. List groups
4. Create new group
5. Join group
6. Set delete time
  
• The client also uses the message structure for receiving the messages from the server.
  
• The message queues are created by the client using a hashing function which generates a
key from the username(every username should be unique).
  
• The client uses two threads. One for sending the requests to the user and another to receive
replies from the user.
  
• Different functions used are:
1. void * output(void *arg): receive the messages from the server and display on the
terminal.
2. void printMenu(): Print the various choices present in the group messaging system.
