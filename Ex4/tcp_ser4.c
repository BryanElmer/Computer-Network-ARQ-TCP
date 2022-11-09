#include "headsock.h"

#define BACKLOG 10
#define BATCH_SIZE 4

void str_ser(int sockfd);

int main(void) {
    pid_t pid;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;

    int sockfd, con_fd, ret;
	int sin_size;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("Error in creating socket!\n");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYTCP_PORT);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("172.0.0.1");
	bzero(&(my_addr.sin_zero), 8);

    // Binding socket
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));
	if (ret < 0) {
		printf("Error in binding socket!\n");
		exit(1);
	}
	
    // Listen socket
	ret = listen(sockfd, BACKLOG);
	if (ret < 0) {
		printf("Error in listening!\n");
		exit(1);
	}

    while(1) {
        printf("Waiting for data\n");
        sin_size = sizeof(struct sockaddr_in);

        // Accepting connection
		con_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (con_fd < 0) {
			printf("Error in accepting connection!\n");
			exit(1);
		}

        // Generate acception process
		if ((pid = fork()) == 0) {
			close(sockfd);
			str_ser(con_fd);
			close(con_fd);
			exit(0);
		} else {
            close(con_fd); // Closing parent process
        }
    }
    close(sockfd);
    exit(0);
}

void str_ser(int sockfd) {
	FILE *fp;
	struct pack_so recvPacket;
	struct ack_so ack;

    char buff[BUFSIZE];
	int end, n;
	long lseek = 0;
	end = 0;
    n = 0;

    printf("Receiving data!\n");

    while(!end) {
        // Receive packet
        if ((n = recv(sockfd, &recvPacket, PACKLEN, 0)) == -1) {
			printf("Error when receiving!\n");
			exit(1);
		}

        // Checks if end of file
		n = recvPacket.len;
		if (recvPacket.data[n - 1] == '\0') {
			end = 1;
			n--;
		}
		memcpy((buff + lseek), recvPacket.data, n);
		lseek += n;

        if (recvPacket.num == BATCH_SIZE || end == 1) {  // Sends ack after every batch
            ack.num = 1;
            ack.len = 1;
            if ((n = send(sockfd, &ack, 2, 0)) == -1) {
                printf("Send ack error!\n");
                exit(1);
            }
        }
    }

    // Opening file to write to
    if ((fp = fopen ("myTCPreceive.txt","wt")) == NULL) {
		printf("File doesn't exist\n");
		exit(0);
	}
	fwrite (buff , 1 , lseek , fp);
	fclose(fp);
	printf("A file has been successfully received!\nThe total data received is %d bytes\n", (int)lseek);
}