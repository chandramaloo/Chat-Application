#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
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
	
	//declarations
	int PORT=atoi(argv[1]);
	fd_set all_fds, read_fds;
	FD_ZERO(&all_fds); 
	FD_ZERO(&read_fds);

	struct sockaddr_in my_addr, their_addr, list_addr; 
	
	int fdmax;
	int BACKLOG=10;
	int new_fd;
	char buf[256];
	int nbytes;
	int addrlen;
	int i, j, k;
	int size = sizeof(struct sockaddr);
	//creating socket
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd==-1) perror("socket");
	my_addr.sin_family = AF_INET; 
	my_addr.sin_port = htons(PORT); 
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero), '\0', 8); 
	
	//binding
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))== -1) {
		perror("bind");
		exit(2);
	}
	//making ports reusable
	int yes=1; 
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(3);
	}	
	//listening
	if (listen(sockfd,BACKLOG) == -1) {
		perror("listen");
		exit(3);
	}
	//adding initial socket
	FD_SET(sockfd, &all_fds);
	fdmax = sockfd; 
	while(1){
		//copying sockets descriptors
		read_fds = all_fds;
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(3);
		}
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { //if socket exists 
				if (i == sockfd) {	//new connection, listened on original socket
					addrlen = sizeof(struct sockaddr);
					if ((new_fd = accept(sockfd, &their_addr, &addrlen)) == -1) {
						perror("accept");
					} 
					else {
						FD_SET(new_fd, &all_fds); 
						if (new_fd > fdmax) { 
							fdmax = new_fd;
						}
					}
				}
				else {
					//receving from socket
					if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
						perror("recv");
						close(i); 
						FD_CLR(i, &all_fds);
					}
					else {
						char msg[1024];
						getpeername(i, (struct sockaddr *)&their_addr, &size);
						//"Bye" condition
						if(buf[0]=='B' && buf[1]=='y' && buf[2]=='e' && buf[3]=='\n'){
							sprintf(msg,"Goodbye %s:%d",inet_ntoa(their_addr.sin_addr),ntohs(their_addr.sin_port));
							int len, bytes_sent;
							len = strlen(msg);
							printf("%s \n",msg);
							bytes_sent = send(i, msg, len, 0);
							//closed socket
							close(i);
							FD_CLR(i, &all_fds);
						}
						//"List" condition
						else if(buf[0]=='L' && buf[1]=='i' && buf[2]=='s' && buf[3]=='t' && buf[4]=='\n'){
							sprintf(msg,"OK %s:%d",inet_ntoa(their_addr.sin_addr),ntohs(their_addr.sin_port));
							for(k = 0; k <= fdmax; k++){
								if (FD_ISSET(k, &all_fds) && k!=i && k!=sockfd){
									getpeername(k, (struct sockaddr *)&list_addr, &size);
									char temp[1024];
									sprintf(temp,", %s:%d",inet_ntoa(list_addr.sin_addr),ntohs(list_addr.sin_port));
									strcat(msg,temp);
								}
							}
							int len, bytes_sent;
							len = strlen(msg);
							printf("%s \n",msg);
							bytes_sent = send(i, msg, len, 0);
						}
						//Normal messages
						else{
							sprintf(msg,"OK %s:%d",inet_ntoa(their_addr.sin_addr),ntohs(their_addr.sin_port));
							int len, bytes_sent;
							len = strlen(msg);
							printf("%s \n",msg);
							bytes_sent = send(i, msg, len, 0);
						}  
					}
				} 
			}
		}
	}
	return 0;
}
