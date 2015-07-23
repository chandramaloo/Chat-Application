 #include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_INPUT_SIZE 100
int BACKLOG = 10;

int main(int argc, char *argv[]){
	int sockfd;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	char inputbuf[MAX_INPUT_SIZE];
    
    //handling the errors in arguments to program
    if (argc != 2) {
		//check for number of arguments
		fprintf(stderr,"usage: %s <tcp-port-number>\n", argv[0]);
		exit(1);
    }
    else{
		int i=0;
		if(argv[1][0] == '-'){
			//check fornegative number
			fprintf(stderr,"Invalid argument for <tcp-port-number>\n");
            exit(1);
		}
		for(; argv[1][i] != 0; i++){
			if(!isdigit(argv[1][i])){
				//check for integer value
				fprintf(stderr,"Invalid argument for <tcp-port-number>\n");
				exit(1);
		   }
	   }
    }
    int MYPORT = atoi(argv[1]);
	
    //creating socket
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd==-1) perror("socket");
	my_addr.sin_family = AF_INET; 
	my_addr.sin_port = htons(MYPORT); 
	my_addr.sin_addr.s_addr = INADDR_ANY;
	printf("Address is %s \n",inet_ntoa(my_addr.sin_addr));
	memset(&(my_addr.sin_zero), '\0', 8);
	int yes=1;
	//making the port reusable
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(3);
	}
	//bind
	if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))==-1){
		perror("bind");
		exit(2);
	}
	//listening on port
	listen(sockfd,BACKLOG);
	//accepting connection
	int sin_size = sizeof(struct sockaddr_in);
	int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	if(new_fd== -1){
		perror("accept");
		exit(3);
	}
	else close(sockfd); //close once one connection is accepted
	//receiving message continuously
	while(1){
		char buf[1024];
		strcpy(buf,"");
		int MAXBUFFER=1024,FLG=0;
		if(recv(new_fd,buf,MAXBUFFER,FLG)<=0){
			perror("recv");
		}
		else{
			char msg[1024];
			// "Bye" condition
			if(buf[0]=='B' && buf[1]=='y' && buf[2]=='e' && buf[3]=='\n'){
				sprintf(msg,"Goodbye %s:%d",inet_ntoa(their_addr.sin_addr),ntohs(their_addr.sin_port));
				int len, bytes_sent;
				len = strlen(msg);
				printf("%s \n",msg);
				bytes_sent = send(new_fd, msg, len, 0);
				// close the connection
				close(new_fd);
				exit(0);
			}
			else{
				sprintf(msg,"OK %s:%d",inet_ntoa(their_addr.sin_addr),ntohs(their_addr.sin_port));
				int len, bytes_sent;
				len = strlen(msg);
				printf("%s \n",msg);
				bytes_sent = send(new_fd, msg, len, 0);
			}  
		 }
	}
	return 0;
}
