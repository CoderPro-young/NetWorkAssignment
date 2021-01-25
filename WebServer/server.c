#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>


#define MSGSIZE 1460 
#define MAXLINE 1024
#define LISTENQ 1024 
#define SERVER_STRING "Server: Xuxy-TinyHttpd/0.1\r\n"
#define HTTPPORT 2021 

int read_line(int fd, char buf[], int MAXSIZE){
	int i = 0;
	char c;  
	int n = 0;
	//printf("fd is %d \n", fd);  
	while(i < MAXSIZE){
		if(recv(fd, &c, 1, 0) < 0){
			perror("recv"); 
			return -1; 
		}
		printf("get %c \n", c); 
		/*	
		if(recv(fd, &c, 1, 0) < 0){
			perror("recv")	; 
			return -1; 	
		}
		*/	
		if(c == '\n'){
			buf[i++] = '\0'; 
			n++; 
			break; 
		}
		buf[i++] = c; 		
		n++; 
	}
	
	return n; 

}

int parse_url(const char url[], char filename[]){
	/*
		parse right filename: return 1
		parse wrong         : return 0 
	*/
	struct stat sbuf; 
	strcpy(filename, "."); 
	strcat(filename, url); 
	/* filename is a dir name */
	/*
		stat(); 
		if(...)
	*/
	if(url[strlen(url) - 1] == '/'){
		strcat(filename, "index.html"); 
	}
	if(stat(filename, &sbuf) < 0){
		return 0; 
	}	
	if((sbuf.st_mode & S_IFMT) == S_IFDIR){
		strcat(filename, "/index.html"); 
	}
	return 1; 
}

void not_found(int client){
    char buf[1024];

    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "your request because the resource specified\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
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

void read_requesthead(int connfd){
	char buf[MAXLINE]; 
	while((read_line(connfd, buf, MAXLINE)) > 0 && strcmp(buf, "\r\n")){
		printf("buf is %s \n"); 	
	}
	return ; 
}

void* deal_request(void* arg){
	int connfd = *((int*)arg);
	printf("connfd is %d \n", connfd);  
	int is_static = 0; 
	char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE]; 
	char filename[MAXLINE]; 
	struct stat sbuf; 

	read_line(connfd, buf, MAXLINE); 
	printf("buf is %s \n", buf); 
	sscanf(buf, "%s %s %s", method, url, version); 
	printf("url is %s \n", url); 
	if(strcmp(method, "GET") == 0){
		is_static = parse_url(url, filename);
		printf("filename is %s \n", filename);  
		if(is_static){
			if(stat(filename, &sbuf) < 0){
				not_found(connfd);
				read_requesthead(connfd);  
				return (void*)(0); 
			}
			if((sbuf.st_mode & S_IFMT) != S_IFREG || !(S_IRUSR & sbuf.st_mode)){
				cannot_read(connfd, filename); 
			}
		}
		else{
			read_requesthead(connfd); 
		}
		serve_static(connfd, filename); 
	}
	close(connfd); 
	
	return (void*)(0); 
}

int init_serverSocket(){
	int servefd;
	int on = 1;  
	servefd = socket(AF_INET, SOCK_STREAM, 0); 
	
	struct sockaddr_in sockaddr; 
	socklen_t socklen = sizeof(sockaddr); 
	bzero(&sockaddr, sizeof(sockaddr)); 

	sockaddr.sin_family = AF_INET; 
	sockaddr.sin_port = htons(HTTPPORT); 
	inet_pton(AF_INET, "0.0.0.0", &sockaddr.sin_addr); 
	
	if((setsockopt(servefd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0){
		perror("setsockopt"); 
		exit(1); 
	}
	if((bind(servefd, (struct sockaddr*)&sockaddr, socklen)) < 0){
		perror("bind"); 
		exit(1); 
	}
	
	if(listen(servefd, LISTENQ) < 0){
		perror("listen"); 
		exit(1); 
	}
	
	return servefd; 
}


int main()
{
	int serverSocket, connectionSocket; 
	/* prepare a server socket */ 
	serverSocket = init_serverSocket(); 
	struct sockaddr_in connaddr;
	socklen_t connaddrlen = sizeof(connaddr);  
	while(1){
		// establish the connection
		printf("ready to serve\n"); 
		bzero(&connaddr, sizeof(connaddr)); 
		connectionSocket = accept(serverSocket, (struct sockaddr*)&connaddr, &connaddrlen);
		if(connectionSocket < 0){
			perror("accept"); 
			exit(1); 
		}
		pthread_t tid; 
		if(pthread_create(&tid, NULL, deal_request, (void*)&connectionSocket) < 0){
			perror("pthread_create"); 
			exit(1); 
		}
	//	close(connectionSocket); 
	}
}
