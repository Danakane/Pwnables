#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

void error(const char *msg)
{
	perror(msg);
	exit(1);
}


void helper() 
{
    asm("pop %rdi; pop %rsi; pop %rdx; ret");
    system("pause");
}
 

int echofunction()
{
	int errcode = 0;
	char buffer[256];
	memset(buffer, 0, 256);
	errcode = read(0, buffer, 512); //stack-based buffer overflow
	if(0 == strcmp(buffer, "exit\n"))
		return 0;
	fprintf(stderr, buffer); // format-string vulnerability
	//printf(buffer); // doesn't work as expected
	return 1;
}

int main(int argc, char* argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	pid_t childpid = -1;
	if(argc < 2)
	{
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if(bind(sockfd, (struct sockaddr *) & serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	while(-1 != (newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)))
	{
		childpid = fork();
		if(0 > childpid)
			error("ERROR on fork");
		if (0 < childpid)
			close(newsockfd); //parent doesn't need the socket
		if(0 == childpid)
		{
			// fork without execve() : Complete copy of the parent process => Possible to brute-force
			/* Redirect stdin/stdout on newsockfd*/
			char greet[] = "ovrflwmyechoserverv0.1 : welcome!\n";
			write(newsockfd, greet, strlen(greet));
			dup2(newsockfd, STDIN_FILENO);  /* duplicate socket on stdin too */
			dup2(newsockfd, STDERR_FILENO);  /* duplicate socket on stderr */
			dup2(newsockfd, STDOUT_FILENO);  /* duplicate socket on stdout but doesn't works :(*/
			close(newsockfd);  /* can close the original after it's duplicated */
			while(1 == echofunction())
				;
			break;
		}
	}
	return 0;
}
