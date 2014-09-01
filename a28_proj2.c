/* Name - Aditya Singla*/
/* Email - a28@buffalo.edu*/
/* References - Beej's Guide to socket programming*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<malloc.h>
#include<sys/stat.h>
#include<fcntl.h>

FILE * pfile;

struct server
{
	
	char ip[INET6_ADDRSTRLEN];
	char port[10];
};

typedef struct server * serv;

serv servers;

struct vector_table
{
	int isNeighbour;
	int isNeighbourDisabled;
  	int cost[5]; 	
};

struct vector_table * vector;

//Declarations for file descriptior sets
fd_set read_file_desc; 
fd_set write_file_desc;
fd_set error_file_desc;
fd_set serv_fd;
fd_set client_fd;
fd_set master_fd;

//Declarations for maximum value of file descriptors
int read_file_desc_max;
int write_file_desc_max;
int error_file_desc_max;
int fd_max;

int disable[5];


//ip if the machine
char myIp[INET6_ADDRSTRLEN];

//port number of the machine
char portNum[10];

//hostname of the machine
char hostName[50];

//id for the server
int myId;

//total number of servers
int numServers;

//total numbers of neighbours
uint16_t numNeighbours;

//listening socket of the machine
int socket_fd;

//other variables
int messageBytes;
int interval;
int numPackets;


void set_write_file_desc(int file_desc)
{
	FD_SET(file_desc, &write_file_desc);
	if (write_file_desc_max <= file_desc)
	{
		write_file_desc_max = ++file_desc;
	}
}

//function for setting error file descriptor set
void set_error_file_desc(int file_desc)
{
	FD_SET(file_desc, &error_file_desc);
	if (error_file_desc_max <= file_desc)
	{
		error_file_desc_max = ++file_desc;
	}
}

//function for setting master file descriptor set
void set_master_fd(int fd)
{
	FD_SET(fd,&master_fd);
	if(fd_max <= fd)
	{
		fd_max = ++fd;
	}
}

//setting file descriptor set for server fd
void set_serv_fd(int fd)
{
	FD_SET(fd,&serv_fd);
	set_master_fd(fd);
}

//setting file descriptor for all client fd
void set_client_fd(int fd)
{
	FD_SET(fd,&client_fd);
	set_master_fd(fd);
}

//create socket for recieving packets
void createSocket(char *portSock)
{
	struct addrinfo input;
	struct addrinfo *response;
	struct addrinfo *iterator;
	int result;
	int sock;
	int yes = 1;
	memset(&input,0,sizeof input);
	input.ai_family = AF_UNSPEC;
	input.ai_socktype = SOCK_DGRAM;
	input.ai_flags = AI_PASSIVE;
	
	
	if((result = getaddrinfo(NULL,portSock,&input,&response)) == 0)
	{
		for(iterator=response;iterator != NULL;iterator=iterator->ai_next)
		{
			if((sock=socket(iterator->ai_family,iterator->ai_socktype,iterator->ai_protocol)) == -1)
			{
				perror("port not available. Please try running program with another port");
				
				exit(1);
			}

			
			if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))== -1)
			{
				perror("setsockop");
				exit(1);

			}

			if((result =bind(sock,iterator->ai_addr,iterator->ai_addrlen))== -1)
			{
				perror("bind");
				close(sock);
				
				exit(1);
			}		
			if(sock == 0)
			{
				printf("Machine already listening to the port. Please run the program with a different port number\n ");
	exit(0);
			}
			socket_fd = sock;
			set_client_fd(socket_fd);
			printf("Machine waiting for messages:%s\n",portSock);	
			break;
		}
	}
 			freeaddrinfo(response);
			

			
}

//get the ip of the machine
void get_my_ip()
{
	struct addrinfo input;//input to getaddrinfo
	struct addrinfo *response;	//response from getaddrinfo
	struct addrinfo *iterator;	//iterator for response
	struct sockaddr_in server;	// google addr parameters	
	struct sockaddr name;		  // struct address for system
	socklen_t namelen = sizeof(name); //length of struct address for ip address
	int result;				
	char ipstr[INET6_ADDRSTRLEN];		//string to hold the ip address
	int sock;	
	
	
	
	
	
	const char* serverIpAddress = "8.8.8.8";
	int serverPort = 53;

	socklen_t addr_len;
	memset(&input,0, sizeof input);
	input.ai_family = AF_INET;
	input.ai_socktype = SOCK_DGRAM;

	 result = getaddrinfo("localhost","4567",&input,&response);
	if(result == 0)
	{
		for(iterator = response;iterator!=NULL;iterator = iterator->ai_next)
		{
			
			if((sock = socket(iterator->ai_family,iterator->ai_socktype,iterator->ai_protocol)) == -1)
			{
					perror("socket");	
					continue;		
			}

		    memset(&server, 0, sizeof(server));
		    server.sin_family = AF_INET;
		    server.sin_addr.s_addr = inet_addr(serverIpAddress);
		    server.sin_port = htons(serverPort);

		    int err = connect(sock,(struct sockaddr *)&server,sizeof(server));
		    if(err==-1)
		    {
			perror("error");
		    }
		    
		    err = getsockname(sock, (struct sockaddr*) &name, &namelen);
		    
		    const char *p = inet_ntop(AF_INET, &((struct sockaddr_in *)&name)->sin_addr, ipstr, sizeof ipstr);
		    strcpy(myIp,ipstr);
		    
                    printf("IP Address of this machine is: %s\n",myIp);

		    close(sock);

		    result = gethostname(hostName,sizeof(hostName));

		    freeaddrinfo(response);
		    
		    break;	
		}
		
		
	}
	else
	{
		fprintf(stderr,"%s\n",gai_strerror(result));
		exit(0);
	}

}

//remove \n character from the command line input
char* removeLastChar(char *commandText)
{
   	int i;
	int length;
    	char *modifiedCommand;

   	
 	length = strlen(commandText);
	modifiedCommand = (char *)malloc(length-1);
 
    	for(i = 0; i < length-1; i++)
    	{
        	modifiedCommand[i] = commandText[i]; 
    	}

	modifiedCommand[length-1] = '\0';
 

        return modifiedCommand;
}

unsigned char * minCost(int serv)
{
	uint16_t cost = 9999;	
	unsigned char * minCost;
	minCost = malloc(2);
	int i;
	
	for(i=0;i<numServers;i++)
	{
		if((i+1 != myId) && (cost > vector[serv - 1].cost[i]))
		{
			cost = vector[serv - 1].cost[i];
		}
	}
	memcpy(minCost,&cost,2);
	return minCost;
}

//distance vector algorithm
void runAlgorithm(unsigned char * messageValue)
{
	
	unsigned char messageText[messageBytes];
	memcpy(messageText,messageValue,messageBytes);
	char ip[INET6_ADDRSTRLEN];
	int id;
	uint16_t cost;
	uint16_t noUpdates;
	int k = 8;
	int j = 0;
	
	sprintf(ip,"%d.%d.%d.%d",messageText[4],messageText[5],messageText[6],messageText[7]);
	
	
	int i = 0;
	for(i=0;i<numServers;i++)
	{
		if(strcmp(ip,servers[i].ip) == 0)
		{
			id = i+1;
			break;
		}
	}
	if(vector[id-1].isNeighbour == 1 && vector[id-1].isNeighbourDisabled == 0)
	{
		struct timeval end_time;
		gettimeofday(&end_time,NULL);
		disable[id-1] = end_time.tv_sec;
		printf("Recieved a message from server:%d\n",id);
		
		memcpy(&noUpdates,messageText,2);
		for(i=0;i<numServers;i++)
		{
			uint16_t updId;
			uint16_t minCo;
			k = k + 8;
			memcpy(&updId,messageText+ k,2);
			memcpy(&minCo,messageText + k + 2,2);
			k = k + 4;
			if(updId != myId && updId != id)
			{
				if(minCo + vector[id -1].cost[id-1] < 9999)
				{
					vector[updId -1].cost[id-1] = vector[id -1].cost[id-1] + minCo;
				}
			else																		
				{
					vector[updId -1].cost[id-1] = 9999;
				}
				
			} 
			
		}
		
			
	}	
	
}	

//display the distance vector for a server
void display()
{
	int i = 0;	
	printf("%6s\t%11s\t%5s\n","Source","Destination","Cost");
	for(i=0;i<numServers;i++)
	{
		uint16_t cost;
		memcpy(&cost,minCost(i+1),2);
		if(cost >= 9999 )
		{
	     		printf("%6d\t%11d\t%5s\n",myId,i+1,"inf");
		}
		else
		{
			printf("%6d\t%11d\t%5d\n",myId,i+1,cost);
		}
	}
}

//disable a server
void disableServer(char * serverId)
{
	int server = atoi(serverId);
	
	int j = 0;
	if(vector[server-1].isNeighbour == 0)
	{
		printf("Server not a neighbour.\n");
	}
	else
	{
		vector[server-1].isNeighbourDisabled = 1;
		for(j=0 ; j<numServers ;j++ )
		{
			if(j+1 != myId)
			{
				vector[j].cost[server-1] = 9999;
			}
		}
		printf("DISABLE:SUCCESS\n");
	}	
	
}	

//update cost for a link
void update(char *source, char *dest, char *cst)
{
	int src;
	int des;
	int i = 0;
	src = atoi(source);
	des = atoi(dest);
	if(myId != src)
	{
		printf("The server can only update its own distance vectors\n");
	}
	else if(vector[des -1].isNeighbour == 0)
	{
		printf("The destination server id is not a neighbour\n");
	}
	else
	{
		
		if(strcmp(cst,"inf") == 0)
		{	
			vector[des -1].isNeighbourDisabled = 0;
			struct timeval start_time;
			gettimeofday(&start_time,NULL);
			int i = 0;
			disable[des -1] = start_time.tv_sec;
				
			for(i=0;i<numServers;i++)
			{
				if(i+1!=myId)
				{
					vector[i].cost[des-1] = 9999;
				}
				
			}
			
		
		}
		else
		{
			int cost = atoi(cst);
			if(vector[des -1].isNeighbourDisabled = 1)
			{
				vector[des-1].cost[des-1] = cost;
				vector[des -1].isNeighbourDisabled = 0;
				struct timeval start_time;
				gettimeofday(&start_time,NULL);
				int i = 0;
				disable[des -1] = start_time.tv_sec;
	
			}
			else
			{
				
				uint16_t lastCost;
				lastCost = vector[des-1].cost[des-1];			
				vector[des-1].cost[des-1] = cost;
				uint16_t min;
				for(i = 0;i<numServers;i++)
				{
						if(i+1!=myId && i+1!=des-1)
					{
						vector[i].cost[des-1] =  vector[i].cost[des-1] - lastCost + cost;
					}
				}		
		
			}
			
		}
		printf("UPDATE:SUCCESS\n");
	}
}
	
//scanning of command entered by the user
void executecommand(char * commandText)
{
	int inc=0;
	
	char * modifiedCommand = removeLastChar(commandText);	

	char *command[4];
	
	char * str_tok;
	
	str_tok = strtok(modifiedCommand," ");
	
	while(str_tok!=NULL)
	{
		command[inc++] = str_tok;
		
		str_tok = strtok(NULL," ");
		
	}	
		
	if(strcmp("EXIT",command[0]) == 0||strcmp("exit",command[0]) == 0)
	{
		exit(0);
	}
	else if(strcmp("UPDATE",command[0])==0||strcmp("update",command[0]) == 0)
	{
		if(inc == 4)
		{
			update(command[1],command[2],command[3]);
			
		}
		else
		{
			printf("Invalid command format.\n");
			
		}
	}
	else if(strcmp("CREATOR",command[0]) == 0 ||strcmp("creator",command[0]) == 0)
	{
		printf("Name:Aditya Singla\nUBITNAME:a28\nUB email address:a28@buffalo.edu\n");

	}
	else if(strcmp("HELP",command[0]) == 0 ||strcmp("help",command[0]) == 0)
	{
		printf("Commands:\n1)DISPLAY: Displays the distance vector for the server.\n2)UPDATE <sourceid> <destid> <cost>: Update the cost for a particular nieghbour.\n3)PACKETS: Displays the number of packets recieved since the last time the command was called.\n4)DISABLE <server id>: Disbale a neighbour.\n5)CRASH: Will crash the program. No packets or commands will be executed.\n6)CREATER: Displays the ubit and name of the student\n7)EXIT: To exit the program.\n");

	}
	else if(strcmp("PACKETS",command[0]) == 0 ||strcmp("packets",command[0]) == 0)
	{
		
		printf("Number of packets recieved since last command:%d\n",numPackets);
		numPackets = 0;
		
	}
	else if(strcmp("DISABLE",command[0]) == 0 ||strcmp("disable",command[0]) == 0)
	{
		if(inc == 2)
		{
			disableServer(command[1]);
		}
		else
		{
			printf("Invalid command format.\n");
			
		}
		
	}
	else if(strcmp("DISPLAY",command[0]) == 0 ||strcmp("display",command[0]) == 0)
	{
		display();
	}
	else
	{
		printf("Invalid command format.\n");
	}
	
}

//prepare message for sending to neighbours
unsigned char outputIp[4];
unsigned char * prepareIp(char * inputIp)
{
	
	char ip[INET6_ADDRSTRLEN];
	memcpy(ip,inputIp,sizeof(ip));
	memset(outputIp, 0x00,sizeof(outputIp));
	
	char * str_tok;
	
	int inc = 0;
	uint8_t part;

	str_tok = strtok(ip,".");
	
	while(str_tok!=NULL)
	{
		part  = atoi(str_tok);
		outputIp[inc++] = part;		
		str_tok = strtok(NULL,".");
		
	}

	return outputIp;	
}

//prepare message for sending to neighbours
unsigned char * prepareMessage()
{
	unsigned char * message;
	uint16_t por;
	uint16_t id;
	
	por = atoi(portNum);
	
	int i = 0;
	message = malloc(8 + 12 * numServers);
	memcpy(message,&numServers,sizeof(numServers));
	memcpy(message+2,&por,sizeof(por));
	memcpy(message+4,prepareIp(myIp),4);

	int k = 8;
	for (i=0;i<numServers;i++)
	{

		memcpy(message+k,prepareIp(servers[i].ip),4);
		k = k+4;
		por = atoi(servers[i].port);
		memcpy(message+k,&por,sizeof(por));
		id = i+1;	
		k = k+4;
		memcpy(message + k,&id,sizeof(id));
		k = k+2;
		memcpy(message+k,minCost(i+1),2);
		k = k+2;
	}
		
	
	return message;
} 

//function for disabling a server when intereval increases threshold(interval * 3) 
void initializeDisable()
{
	struct timeval start_time;
	gettimeofday(&start_time,NULL);
	int i = 0;
	for(i = 0; i<numServers;i++)
	{
		disable[i] = start_time.tv_sec;
	}
}

//check whether updated are coming from a server
void checkToDisable()
{
	struct timeval end_time;
	gettimeofday(&end_time,NULL);

	int i = 0;

	int j = 0;

	for(i = 0; i<numServers;i++)
	{
		if(i+1 != myId)
		{
			if(vector[i].isNeighbour == 1 && vector[i].isNeighbourDisabled == 0)
			{
				
				if((disable[i] + 3*interval) <= end_time.tv_sec)
				{
					vector[i].isNeighbourDisabled = 1;
					for(j=0 ; j<numServers ;j++ )
					{
						if(j +1 != myId)
						{	
							vector[j].cost[i] = 9999;
						}
					}
					printf("Server %d cost updated to infinity. Please call update to enable again\n",i+1);
				}
			}
		}	
	}
}

// main function
void main(int args, char * argv[])
{

	char * buf = NULL;
	int i = 0;
	size_t  s= 0;
	struct sockaddr_storage their_addr;
        socklen_t addr_len;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int sock_fd;
	char line[100];
	FILE *fp;
	int cur_data = 0;
	char *topology[20];
	int j;


		
	//Check whether number of arguments are valid or not
	if(args != 5)
	{
		printf("Please enter arguments correctly:./proj -t <topology-filename> -i <interval-for-updates>\n");
		exit(0);
	}

	if(strcmp(argv[1],"-t") != 0)  
	{
		
		printf("Please enter arguments correctly:./proj -t <topology-filename> -i <interval-for-updates>\n");
		exit(0);
		
	}

	if(strcmp(argv[2],"") == 0)
	{
		printf("Please mention the topology filename.\n");
		exit(0);
	}

	if(strcmp(argv[3],"-i") != 0)
	{
		printf("Please enter arguments correctly:./proj -t <topology-filename> -i <interval-for-updates>\n");
		exit(0);
	}

	if(strcmp(argv[4],"") == 0)
	{
		printf("Please mention the interval a28.\n");
		exit(0);
	}
	

	interval = atoi(argv[4]);

	FD_ZERO(&read_file_desc);
	
	FD_ZERO(&write_file_desc);
	
	FD_ZERO(&error_file_desc);

	FD_ZERO(&serv_fd);
	
	set_serv_fd(0);

	set_write_file_desc(1);

	set_error_file_desc(2);
	
	get_my_ip();	
	
		
	
	int k = 0;

	fp = fopen(argv[2],"r");
	
	i =0;

	while (fgets(line, sizeof line, fp) != NULL)
	{
		topology[i] = malloc(100);
		strcpy(topology[i],removeLastChar(line));
		i++;	
	}
	fclose(fp);
	
	k = i;
	
	numServers = atoi(topology[0]);

	servers	= malloc (numServers * sizeof(struct server));

	vector = malloc (numServers * sizeof(struct vector_table));

	for(i=0;i<numServers;i++)
	{
	   	for(j=0;j<numServers;j++)
	   	{
	     		vector[i].cost[j] = 0;
	   	}	   
	}

	i = 0;
	
	while(i <= k)
	{
	    
	    if(i == 0)
	    {

		numServers = atoi(topology[i]);
            }
            else if(i == 1)
	    {
				
		numNeighbours = atoi(topology[i]);
            }
	    else if(i >= 2 && i <=numServers + 1)
	    {
				
		int inc=0;
	
		int con = 0;

		char * tokenized[3];
	
		char * str_tok;
	
		str_tok = strtok(topology[i]," ");
	
		while(str_tok!=NULL)
		{
			tokenized[inc++] = str_tok;
		
			str_tok = strtok(NULL," ");
		
		}

		con = atoi(tokenized[0]);
		
		strcpy(servers[con - 1].ip,tokenized[1]);
		
		if(strcmp(myIp,tokenized[1]) == 0)
		{
			myId = con;
			strcpy(portNum,tokenized[2]);
			
		}
		strcpy(servers[con - 1].port,tokenized[2]);
		
            }	
	    else 
	    {
				
		int inc=0;
			
		char * tokenized[3];
	
		char * str_tok;

		int con = 0;
	
		str_tok = strtok(topology[i]," ");
	
		while(str_tok!=NULL)
		{
			tokenized[inc++] = str_tok;
			str_tok = strtok(NULL," ");
		}

		con = atoi(tokenized[1]);		
		vector[con - 1].isNeighbour = 1;
		vector[con - 1].isNeighbourDisabled = 0;
		vector[con - 1].cost[con - 1] = atoi(tokenized[2]);
	    }
	
		i++;
		
	}


	messageBytes = 8 + 12*numServers;


	for(i=0;i<numServers;i++)
	{
	 if(i+1 != myId)
	 {
	   for(j=0;j<numServers;j++)
	   	{
	     		if(vector[i].cost[j] == 0 && (j + 1 != myId))
	     		{
				vector[i].cost[j] = 9999;
             		}		
	   	}
	   }
	}
	
	display();
	createSocket(portNum);
	i = 0;
	int sendBytes;	
	initializeDisable();
	for(;;)
	{
		
		struct timeval start_time,end_time;
		
		gettimeofday(&start_time,NULL);
			
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		int stepTime;
		stepTime = start_time.tv_sec + interval;
		
		for(;;)
		{
			gettimeofday(&end_time,NULL);

			if(stepTime <= end_time.tv_sec)
			{
				break;
			}

			read_file_desc = master_fd;
			if(select(fd_max,&read_file_desc,NULL,NULL,&tv) == -1)
			{
				perror("select");
			}
		
		
			for(i=0;i<fd_max;i++)
			{
			
				if(FD_ISSET(i,&read_file_desc))
				{
					if(i==0)
					{
						buf = NULL;
						getline(&buf,&s,stdin);
						if(strcmp("\n",buf) == 0)
						{
							printf("Invalid Command\n");
						}
						else if(strcmp("STEP\n",buf) == 0 || strcmp("step\n",buf) == 0)
						{
							stepTime = stepTime - interval;
							printf("STEP:SUCCESS\n");
						}
						else if(strcmp("CRASH\n",buf) == 0 || strcmp("crash\n",buf) == 0)
						{
							while(1)
							{
								
							}
						}
						else
						{
							executecommand(buf);
						}
					}
					else if(i == socket_fd)
					{
						unsigned char read_buf[messageBytes];
						memset(read_buf,0,messageBytes);
						size_t size = 0;
						int nbytes;
						addr_len = sizeof their_addr;				 
						if((nbytes = recvfrom(i,read_buf,messageBytes,0,(struct sockaddr *)&their_addr,&addr_len)) == -1)
						{
							perror("recieve");	
						}
						else
						{
							if(nbytes > 0)
							{
								numPackets++;				
						
								runAlgorithm(read_buf);
									
							}
						}
							
						
						
					}
	

				}
			}		
		}
		checkToDisable();
		for (i = 0;i<numServers;i++)
		{		
			if(strcmp(servers[i].ip,myIp) != 0 && vector[i].isNeighbour == 1 && vector[i].isNeighbourDisabled == 0)
			{
				memset(&hints, 0, sizeof hints);
				hints.ai_family = AF_INET;
				hints.ai_socktype = SOCK_DGRAM;

				rv = getaddrinfo(servers[i].ip,servers[i].port,&hints,&servinfo);
		
				for(p=servinfo;p!=NULL;p=p->ai_next)
				{
					if((sock_fd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
					{
						perror("socket");
						continue;
					}
					break;
				}

				unsigned char *preparedMessage;
				preparedMessage = prepareMessage();
				unsigned char mess[messageBytes];		
				memset(mess,0,messageBytes);
				memcpy(mess,preparedMessage,messageBytes);
				if((sendBytes = sendto(sock_fd,mess,messageBytes,0,p->ai_addr,p->ai_addrlen)) == -1)
				{
					perror("send to");
					exit(1);
				}
				close(sock_fd);
				free(preparedMessage);

				
			}
		}
	}
}
	

