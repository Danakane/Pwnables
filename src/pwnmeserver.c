#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/wait.h>
#include <sys/types.h>

#define err_continue 0
#define err_found 1
#define err_exit 2

void handle(int signum)
{
	wait(NULL);
}

int check(int fd)
{
	int errcode = err_continue;
	char buffer[256];
	memset(buffer, 0, 256);
	errcode = read(fd, buffer, 65535); //stack-based buffer overflow
	if(errcode > 0 && 0 == strcmp(buffer, "P@55w0rd\n"))
	{
		errcode = err_found;
	}
	else if(errcode <= 0 || 0 == strcmp(buffer, "exit\n"))
	{
		errcode = err_exit;
	}
	else
	{
		errcode = err_continue;
	}
	return errcode;
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
		fprintf(stderr, "ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if(bind(sockfd, (struct sockaddr *) & serv_addr, sizeof(serv_addr)) < 0)
		fprintf(stderr, "ERROR on binding");
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	while(-1 != (newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)))
	{
		childpid = fork();
		if(0 > childpid)
			fprintf(stderr, "ERROR on fork");
		if (0 < childpid)
		{
			signal(SIGCHLD, handle);
			close(newsockfd); //parent doesn't need the socket
		}
		if(0 == childpid)
		{
			// fork without execve() : Complete copy of the parent process => Possible to brute-force
			/* Redirect stdin/stdout on newsockfd*/
			char greet[] = "pwnmeserver-V1.0 : Try to find the hidden word!\n";
			write(newsockfd, greet, strlen(greet));
			int errcode = err_continue;
			while(errcode == err_continue)
			{
				errcode = check(newsockfd);
				if(errcode == err_continue)
				{
					char response[] = "Try harder :)\n";
					write(newsockfd, response, strlen(response));
				}
				else if(errcode == err_found)
				{
					char response[] = "Congratz you have found the hidden word!\n";
					write(newsockfd, response, strlen(response));
				}
			}
			close(newsockfd);
			break;
		}
	}
	return 0;
}
