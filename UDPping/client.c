#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define MSGSIZE 128 
#define RECV_WAIT_TIME 1 
#define debug 

typedef struct sockaddr SA; 

static void sig_alrm(int t){
	return ; 
}

int Select(int sockfd, int wait_sec)
{
	fd_set fdset; 
	struct timeval timeout; 
	
	FD_ZERO(&fdset); 
	FD_SET(sockfd, &fdset); 
	
	timeout.tv_sec = wait_sec; 
	timeout.tv_usec = 0; 
	
	int ret = select(sockfd+1, &fdset, NULL, NULL, &timeout); 
	
	return ret; 
}

int main()
{
	int clientSocket, serverSocket; 
	clientSocket = socket(AF_INET, SOCK_DGRAM, 0); 

	struct sockaddr_in destSockaddr; 
	bzero(&destSockaddr, sizeof(destSockaddr)); 
	destSockaddr.sin_family = AF_INET; 
	destSockaddr.sin_port = htons(12000); 
	inet_pton(AF_INET, "127.0.0.1", (SA*)&destSockaddr.sin_addr); 
	socklen_t destSocklen = sizeof(destSockaddr); 	

	char msg[MSGSIZE]; 
	time_t begin, end; 
	//signal(SIGALRM, sig_alrm); 
	for(int i = 0; i < 10; i++){
		begin = time(NULL); 
		sprintf(msg, "ping %d %s", i+1, asctime(localtime(&begin)));	 
		sendto(clientSocket, msg, strlen(msg), 0, (SA*)&destSockaddr, destSocklen); 
		//alarm(1); 
		/* set TIME OUT for clientSocket */
		int ret = Select(clientSocket, RECV_WAIT_TIME); 
		#ifdef debug
		printf("ret is %d \n", ret); 
		#endif 
		if(ret > 0){
			recvfrom(clientSocket, msg, MSGSIZE, 0, (SA*)&destSockaddr, &destSocklen); 
			end = time(NULL); 
			printf("seq[%d]: RTT %ds\n", i+1, end-begin); 
		}
		else if(ret == 0){
			printf("seq[%d]: TIEE OUT\n", i+1); 
			continue; 
		}
		/*
		if(recvfrom(clientSocket, msg, MSGSIZE, 0, (SA*)&destSockaddr, &destSocklen) < 0){
		}
		*/
		//end  = time(NULL); 
		//printf("seq[%d]: RTT %ds\n", i+1, end-begin); 
	}
	
	
	exit(0); 
}
