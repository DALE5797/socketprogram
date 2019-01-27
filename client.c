/*
	Compile as follows:
		gcc mj2cli.c -o client
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netdb.h>

int mytotal = 0;

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		printf("usage %s <svr_host> <svr_port> <rem_ipaddr>\n", argv[0]);
		exit(EXIT_FAILURE);
	}


    int sockfd, portnum, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char *host = "group18";
////////////////USED when making the client server//////////////////////////////////////////////////////
	int sockfd2, connfd;
	int portnum2 = 0;
	socklen_t clilen, len;

	struct sockaddr_in serv_addr2, cli_addr;
//////////////////////////////////////////////////////////////////////
	int sockfd3;
	struct sockaddr_in serv_addr3, serv_addr4;

	struct hostent *server2;
//////////////////////////////////////////////////////////////////////
	int i, j, k, end;
	int pro, port = 0;
	char substr[8], buffer[100], buffer2[100];

    portnum = atoi(argv[2]);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error socket open");
		exit(EXIT_FAILURE);
    }

    if ((server = gethostbyname(argv[1])) == NULL)
    {
        printf("Error: no such host");
        exit(EXIT_FAILURE);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portnum);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        perror("Error connecting");
		exit(EXIT_FAILURE);
    }

    while(1)
    {
        // read() prompt from server
        bzero(buffer, sizeof(buffer));
	if ((n = read(sockfd, buffer, sizeof(buffer))) < 0)
        {
            perror("Error reading from socket");
            exit(EXIT_FAILURE);
        }
        buffer[n] = '\0';

        // print prompt
	    printf("%s", buffer);

	    // Do we need to end this client?
	if(strncmp(buffer, "SERVER: Too", 11) == 0)
	{
		break;
	}

        // scan new number from keyboard, stdin
       	bzero(buffer, sizeof(buffer));
        scanf("%s", buffer);

	if(strncmp(buffer, "0", 1) == 0)
	{
		// write() new number to server
        	if((n = write(sockfd, buffer, sizeof(buffer))) < 0)
        	{
                	perror("Error writing to socket");
                	exit(EXIT_FAILURE);
        	}

		 //just read total so we always have the prot number
        	bzero(buffer, sizeof(buffer));
        	if ((n = read(sockfd, buffer, sizeof(buffer))) < 0)
        	{
            		perror("Error reading from socket");
            		exit(EXIT_FAILURE);
        	}
        	buffer[n] = '\0';

		bzero(buffer, sizeof(buffer));
        	if ((n = read(sockfd, buffer, sizeof(buffer))) < 0)
        	{
            		perror("Error reading from socket");
            		exit(EXIT_FAILURE);
        	}
        	buffer[n] = '\0';

		break;
	}

        // write() new number to server
        if((n = write(sockfd, buffer, sizeof(buffer))) < 0)
	{
		perror("Error writing to socket");
		exit(EXIT_FAILURE);
	}

////////////////////////////////////////////////////////////////////////
	//just read total so we always have the prot number
        bzero(buffer, sizeof(buffer));
        if ((n = read(sockfd, buffer, sizeof(buffer))) < 0)
        {
            perror("Error reading from socket");
            exit(EXIT_FAILURE);
        }
        buffer[n] = '\0';
	portnum2 = atoi(buffer);
/////////////////////////////////////////////////////////////////////////

	// read() total
        bzero(buffer, sizeof(buffer));
	if ((n = read(sockfd, buffer, sizeof(buffer))) < 0)
        {
            perror("Error reading from socket");
            exit(EXIT_FAILURE);
        }
        buffer[n] = '\0';
        // print updated total|PORT message
        printf("%s\n", buffer);

	//are you the client-server?
        if(strncmp(buffer, "SERVER Total: 0", 15) == 0)
        {
                printf("You are the Client-Server\n");

		if((sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) == -1) //socket type and see if it worked
        	{
                	perror("Error socket open");
                	exit(EXIT_FAILURE);
        	}

		bzero((char *) &serv_addr2, sizeof(serv_addr2));

		serv_addr2.sin_family = AF_INET;
        	serv_addr2.sin_addr.s_addr = htonl(INADDR_ANY);
        	serv_addr2.sin_port = htons(portnum2);

		printf("port num: %d\n",portnum2);

		int on = 1;
		setsockopt(sockfd2, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

		if(bind(sockfd2,(struct sockaddr *)&serv_addr2, sizeof(serv_addr2)) == -1) //bind and check$
        	{
                	perror("Error bind");
                	exit(EXIT_FAILURE);
        	}

		if(listen(sockfd2, 2) == -1) //listen and check if it worked
        	{
                	perror("Error listen");
                	exit(EXIT_FAILURE);
        	}

		printf("Waiting for connections...\n"); //for testing only
		clilen = sizeof(cli_addr);

		connfd = accept(sockfd2, (struct sockaddr *)&cli_addr, &clilen);
		if(connfd < 0) //error check to see if the connection went through
                {
                        perror("Error connection"); //for testing only
                        exit(EXIT_FAILURE);
                }
		printf("Client Connection accepted\n");


		bzero(buffer, sizeof(buffer));
		if ((n = read(connfd, buffer, sizeof(buffer))) < 0) //changed from sockfd3
                {
                    perror("Error reading from socket");
                    exit(EXIT_FAILURE);
                }
		printf("Other Clients total: %s\n", buffer);
		printf("Disconnecting client and closing server...\n");

		close(connfd);
		close(sockfd2);

		sockfd2 = 0;
		connfd = 0;
        	portnum2 = 0;

		printf("Returning to previous functionality...\n");
        }

	    // if port message read updated total
        if(strncmp(buffer, "SERVER Message: ", 16) == 0) //operation for connecting the two clients
        {
		//getting the port number
		bzero(buffer, sizeof(buffer));
                if ((n = read(sockfd, buffer, sizeof(buffer))) < 0)
                {
                    perror("Error reading from socket");
                    exit(EXIT_FAILURE);
                }

                buffer[n] = '\0';
		port = atoi(buffer);
//		printf("%d\n", port); //print what is ports value

		//getting its last updated total message
        	bzero(buffer, sizeof(buffer));
		if ((n = read(sockfd, buffer, sizeof(buffer))) < 0)
	        {
	            perror("Error reading from socket");
	            exit(EXIT_FAILURE);
	        }
	        buffer[n] = '\0';
	        printf("%s\n", buffer);

		//getting just its last total
		bzero(buffer, sizeof(buffer));
                if ((n = read(sockfd, buffer, sizeof(buffer))) < 0)
                {
                    perror("Error reading from socket");
                    exit(EXIT_FAILURE);
                }
                buffer[n] = '\0';
		mytotal = atoi(buffer);
//		printf("my total: %d\n", mytotal); //see what the total is

		pro = 5;

		break;
        }
    }

    close(sockfd);

    if(pro == 5) //start the process of connecting to the client-server
    {
//	printf("Pro was 5\n"); //just testing if it went through the loop

	//connect to client-server

            if ((sockfd3 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("Error socket open");
                exit(EXIT_FAILURE);
    	    }

	    serv_addr4.sin_family = AF_INET;
	    serv_addr4.sin_addr.s_addr = inet_addr(argv[3]);
	    len = sizeof(struct sockaddr_in);
		bzero(buffer2, sizeof(buffer2));

	    if(getnameinfo((struct sockaddr *) &serv_addr4, len, buffer2, sizeof(buffer2), NULL, 0, NI_NAMEREQD))
	    {
		printf("Error: could not get host name\n");
                exit(EXIT_FAILURE);
	    }

	    printf("host name: %s\n", buffer2);
	    printf("port num: %d\n", port);

	    if ((server2 = gethostbyname(buffer2)) == NULL)
	    {
//		printf("Host name: %s\n", serv_addr3->h_name);
	        printf("Error: no such host\n");
	        exit(EXIT_FAILURE);
    	    }

	    bzero((char *) &serv_addr3, sizeof(serv_addr3));

            serv_addr3.sin_family = AF_INET;
	    inet_pton(AF_INET, argv[3], &serv_addr3.sin_addr.s_addr);
            serv_addr3.sin_port = htons(port);

            if (connect(sockfd3,(struct sockaddr *) &serv_addr3,sizeof(serv_addr3)) < 0)
            {
                perror("Error connecting");
                exit(EXIT_FAILURE);
            }

		printf("Connected to client-server\n");
		printf("Sending total...\n");

		bzero(buffer, sizeof(buffer));
		sprintf(buffer, "%d", mytotal);
		if((n = write(sockfd3, buffer, sizeof(buffer))) < 0)
                {
                        perror("Error writing to socket");
                        exit(EXIT_FAILURE);
                }

		printf("Total sent, disconnecting...\n");
		printf("Ending program\n");

	close(sockfd3);

    }

    // printf("end\n");
   // close(sockfd);
    return 0;
}
