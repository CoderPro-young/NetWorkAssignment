#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define MSGSIZE 128 
typedef struct sockaddr SA; 

static void sig_alrm(int t){
	return ; 
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
	signal(SIGALRM, sig_alrm); 
	for(int i = 0; i < 10; i++){
		begin = time(NULL); 
		sprintf(msg, "ping %d %s", i+1, asctime(localtime(&begin))); 
		sendto(clientSocket, msg, strlen(msg), 0, (SA*)&destSockaddr, destSocklen); 
		alarm(1); 
		if(recvfrom(clientSocket, msg, MSGSIZE, 0, (SA*)&destSockaddr, &destSocklen) < 0){
			printf("seq[%d]: TIEE OUT", i+1); 
			continue; 
		}
		end  = time(NULL); 
		printf("seq[%d]: RTT %ds\n", i+1, end-begin); 
	}
	
	
	exit(0); 
}
