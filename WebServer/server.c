#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define MSGSIZE 1460 
#define MAXLINE 1024  

int read_line(int fd, char buf[], int MAXSIZE){
	int i = 0;
	char c;  
	int n = 0; 
	while(i < MAXSIZE){
		if(recv(fd, &c, 1, 0) < 0){
			perror("recv"); 
			return -1; 
		}
		if(c == '\r'){
			if(recv(fd, &c, 1, 0) < 0){
					
			}	
			if(c == '\n'){
				buf[i++] = '\0'; 
				n++; 
				break; 
			}
		}
		buf[i++] = c; 		
		n++; 
	}
	
	return n; 

}

int parse_url(){


}

void not_found(int fd){
	const char errmsg[] = "404 NOT FOUND"; 
	send(fd, errmsg, sizeof(errmsg), 0); 
}

void cannot_read(int fd, char filename[]){
	char errmsg[MAXLINE]; 
	bzero(errmsg, 0); 
	sprintf(errmsg, "403 %s Forbidden", filename); 
	send(fd, errmsg, sizeof(errmsg), 0); 
}

void header(int fd, const char filename[]){
	char buf[1024];
    	(void)filename;  /* could use filename to determine file type */

    	strcpy(buf, "HTTP/1.0 200 OK\r\n");
    	send(fd, buf, strlen(buf), 0);
    	strcpy(buf, SERVER_STRING);
    	send(fd, buf, strlen(buf), 0);
    	sprintf(buf, "Content-Type: text/html\r\n");
    	send(fd, buf, strlen(buf), 0);
    	strcpy(buf, "\r\n");
    	send(fd, buf, strlen(buf), 0);
}

void body(int fd, FILE* fp){
	char buf[MAXLINE]; 
	int n; 
	while(n = fgets(buf, MAXLINE, fp) > 0){
		send(fd, buf, n, 0); 
	}

}

void serve_static(int fd, char filename[]){
	FILE* fp; 
	fp = fopen(filename, "r"); 
	
	header(fd, filename); 
	body(fd, fp); 
}

void deal_request(void* arg){
	int connfd = (int)arg; 
	int is_static = 0; 
	char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE]; 
	char filename[MAXLINE]; 
	struct stat sbuf; 

	read_line(connfd, buf, MAXLINE); 
	sscanf(buf, "%s %s %s", method, url, version); 

	if(strcmp(method, "GET") == 0){
		is_static = parse_url(url, filename); 
		if(is_static){
			if(stat(filename, &sbuf) < 0){
				not_found(connfd); 
				return ; 
			}
			if(!(S_ISREG()) || !(S_IRUSR & sbuf.st_mode)){
				cannot_read(connfd); 
			}
		}
		serve_static(connfd, filename); ; 
	}

	return ; 
}

int init_serverSocket(){
	int servefd; 
	servefd = socket(AF_INET, SOCK_STREAM, 0); 
	
	struct sockaddr_in sockaddr; 
	socklen_t socklen = sizeof(sockaddr); 
	bzero(&sockaddr, sizeof(sockaddr)); 

	sockaddr.sa_family = AF_INET; 
	sockaddr.sin_port = htons(HTTPPORT); 
	inet_pton(AF_INET, "0.0.0.0", &sockaddr.sin_addr); 

	bind(servefd, (struct sockaddr*)sockaddr, socklen); 

	return servefd; 
}


int main()
{
	int serverSocket, connectionSocket; 
	/* prepare a server socket */ 
	serverSocket = init_serverSocket(); 
	
	while(1){
		// establish the connection
		printf("ready to serve"); 
		connectionSocket = accept();
		if(connectionSocket < 0){
			perror("accept"); 
			exit(1); 
		}
		tid_t tid; 
		if(pthread_create(&tid, NULL, &deal_request, (void*)connectionSocket) < 0){
			perror("pthread_create"); 
			exit(1); 
		}
		close(connectionSocket); 
	}
}
