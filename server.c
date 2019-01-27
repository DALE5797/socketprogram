/*
	Compile as follows:
		gcc mj2srv.c -o server -lpthread
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h> //this might not be needed
#include <sys/time.h> //this might not be needed
#include <arpa/inet.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

int x = 0;
int port = 0;
int noClients = 0;

int port2 = 0; //used when to have to port

pthread_t c1;

char *ip1; //variables that will hold the ip adresses from the cleints
char *ip2;

void *client_request(void *sockfd); //function that will take care of anythig the client needs

int main(int argc, char *argv[])
{
	if(argc < 2) //check to see if there is a port number provided before proceding
	{
		printf("usage %s <svr_port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int sockfd, connfd, portnum; //socket descriptor, client descriptor, used to get the prot number
	socklen_t clilen;

	struct sockaddr_in serv_addr, cli_addr;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) //socket type and see if it worked
	{
		perror("Error socket open");
		exit(EXIT_FAILURE);
	}
	//printf("Socket made\n"); //for testing only

	bzero((char *) &serv_addr, sizeof(serv_addr));

	portnum = atoi(argv[1]); //getting the port number from the argv[1]

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(portnum);

	int on = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	if(bind(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) //bind and check if it did bind
	{
		perror("Error bind");
		exit(EXIT_FAILURE);
	}
	//printf("Binding done\n"); //for testing only

	if(listen(sockfd, 2) == -1) //listen and check if it worked
	{
		perror("Error listen");
		exit(EXIT_FAILURE);
	}

	printf("Waiting for connections...\n"); //for testing only
	clilen = sizeof(cli_addr);

	pthread_t thread_id;

	while((connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)))
	{
		if(connfd < 0) //error check to see if the connection went through
		{
			perror("Error connection"); //for testing only
			exit(EXIT_FAILURE);
		}

		printf("Client Connection accepted\n");

		//increment number of clients
		noClients++;

		if(pthread_create(&thread_id, NULL, client_request, (void*) &connfd) < 0) //if there is an error
		{
			perror("Error creating thread");
			exit(EXIT_FAILURE);
		}

		//printf("Threads made = %d\n", noClients); //for testing only
	}

	return 0;
}

void *client_request(void *sockfd)
{
	int n, client, num, tot = 0;
	char buffer[100];


	int sock = *(int*) sockfd;

	//first thread will be defined as c1, client1
	if(x == 0)
	{
		c1 = pthread_self();
		x++;
	}

	//2nd thread will be client 2
	client = (pthread_equal(c1, pthread_self())) ? 1 : 2;

	printf("Client Handler Assigned\n");

	// disconnect third client
	if(noClients == 3)
	{
		// write() client number exceeded -> disconnect to client
		bzero(buffer, sizeof(buffer));
		sprintf(buffer, "SERVER: Too Many Clients Connected. Disconnecting...\n");
		if((n = write(sock, buffer, sizeof(buffer))) < 0)
		{
			perror("Error writing to socket");
			exit(EXIT_FAILURE);
		}

		printf("Error: too many clients connected\n");

		// one less client
		noClients--;

		//close socket and exit thread
		close(sock);
		pthread_exit(NULL);

		return NULL;
	}


	while(1)
	{
		// write() prompt to client
		bzero(buffer, sizeof(buffer));
		sprintf(buffer, "Enter Client %d Data: ", client);
		if((n = write(sock, buffer, sizeof(buffer))) < 0)
		{
			perror("Error writing to socket");
			exit(EXIT_FAILURE);
		}

		// read() new number from client
		bzero(buffer, sizeof(buffer));
		if ((n = read(sock, buffer, sizeof(buffer))) < 0)
        	{
            		perror("Error reading from socket");
            		exit(EXIT_FAILURE);
        	}
        buffer[n] = '\0';

		num = atoi(buffer);

		//disconnect if clients enters zero
		if(num == 0)
		{
			// write() disconnect to client
			bzero(buffer, sizeof(buffer));
			sprintf(buffer, "Client %d Disconnecting...", client);
			if((n = write(sock, buffer, sizeof(buffer))) < 0)
			{
				perror("Error writing to socket");
				exit(EXIT_FAILURE);
			}

			printf("Client %d Disconnected, Total Reset\n", client);

			//reset total
			tot = 0;

			//one less client 
			noClients--;

			//renew x value for next client to connect
			x = (client == 1) ? 0 : 1;

			//close socket and exit thread
			close(sock);
			pthread_exit(NULL);

			return NULL;
		}

		//add total to client running total
		tot = tot + num;

		//print updated total
		printf("CLIENT %d: %5d - Total: %d\n", client, num, tot);

		if(port != client && port != 0)
		{
			printf("Starting client-server process\n");

			// just write() total to client
                	bzero(buffer, sizeof(buffer));
                	sprintf(buffer, "%d", port2);
                	if((n = write(sock, buffer, sizeof(buffer))) < 0)
                	{
                        	perror("Error writing to socket");
                        	exit(EXIT_FAILURE);
                	}

			bzero(buffer, sizeof(buffer));
			sprintf(buffer, "SERVER Message: PORT %d", port2);
			if((n = write(sock, buffer, sizeof(buffer))) < 0)
			{
				perror("Error writing to socket");
				exit(EXIT_FAILURE);
			}

			//send just the port number to the client
			bzero(buffer, sizeof(buffer));
			sprintf(buffer, "%d", port2);
			if((n = write(sock, buffer, sizeof(buffer))) < 0)
			{
				perror("Error writing to socket");
				exit(EXIT_FAILURE);
			}


			//send the last updated total
			bzero(buffer, sizeof(buffer));
                	sprintf(buffer, "SERVER Total: %d", tot);
                	if((n = write(sock, buffer, sizeof(buffer))) < 0)
                	{
                        	perror("Error writing to socket");
                        	exit(EXIT_FAILURE);
                	}

			//send just the last updated total
                        bzero(buffer, sizeof(buffer));
                        sprintf(buffer, "%d", tot);
                        if((n = write(sock, buffer, sizeof(buffer))) < 0)
                        {
                                perror("Error writing to socket");
                                exit(EXIT_FAILURE);
                        }

			//process to disconnect the client from server
			printf("Disconnecting Client %d, Total Reset\n", client);
			tot = 0;
                        //one less client
                        noClients--;
                        //renew x value for next client to connect
                        x = (client == 1) ? 0 : 1;
			port = 0;
			port2 = 0;
                        //close socket and exit thread
                        close(sock);
                        pthread_exit(NULL);
                        return NULL;
		}

		if(tot > 1025 && tot < 49151)
		{
			port2 = tot; //this is what we will send over to the connecting client
			printf("Sending Client %d Port Data to Client %d, Reset Total\n", client, (client == 1) ? 2 : 1);
			tot = 0;
			port = client;
		}

		else if(tot > 49151)
		{
			bzero(buffer, sizeof(buffer));
			sprintf(buffer, "SERVER Message: PORT %d exceeds max value of 49151, Reseting Total...", tot);
			if((n = write(sock, buffer, sizeof(buffer))) < 0)
			{
				perror("Error writing to socket");
				exit(EXIT_FAILURE);
			}
			tot = 0;
		}

		// just write() total to client
                bzero(buffer, sizeof(buffer));
                sprintf(buffer, "%d", port2);
                if((n = write(sock, buffer, sizeof(buffer))) < 0)
                {
                        perror("Error writing to socket");
                        exit(EXIT_FAILURE);
                }

		// write() total to client
		bzero(buffer, sizeof(buffer));
		sprintf(buffer, "SERVER Total: %d", tot);
		if((n = write(sock, buffer, sizeof(buffer))) < 0)
		{
			perror("Error writing to socket");
			exit(EXIT_FAILURE);
		}
	}
}
