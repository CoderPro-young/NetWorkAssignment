#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

#define BUFSIZE 128

void unix_error(const char* err)
{
	perror(err); 
	exit(1); 
}


int main()
{
	int serverSocket; 
	char buf[BUFSIZE]; 

	serverSocket = socket(AF_INET, SOCK_DGRAM, 0); 

	struct sockaddr_in sockaddr, clisockaddr; 

	socklen_t socklen = sizeof(sockaddr); 
	socklen_t clisocklen; 	

	sockaddr.sin_family = AF_INET; 
	sockaddr.sin_port = htons(12000); 
	inet_pton(serverSocket, "0.0.0.0", &sockaddr.sin_addr); 

	if(bind(serverSocket, (struct sockaddr*)&sockaddr, socklen) < 0){
		unix_error("bind"); 
	}	
	
	while(1){
		srand((unsigned)time( NULL )); 
		int randnum = rand() % 10; 
		recvfrom(serverSocket, buf, BUFSIZE, 0, (struct sockaddr*)&clisockaddr, &clisocklen); 
		/* consider the packet lost */
		if(randnum < 4){
			printf("packet will be lost\n"); 
			continue; 
		}	
		/* otherwise, the server responses */
		printf("recv message: %s\n", buf); 
		sendto(serverSocket, buf, BUFSIZE, 0, (struct sockaddr*)&clisockaddr, clisocklen); 
		
	}
	return 0; 
}
