#include "headsock.h"

#define BACKLOG 10
#define BATCH_SIZE 2

void str_ser(int sockfd);

int main(void) {
    int sockfd, con_fd, ret;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int sin_size;

    pid_t pid;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("error in socket!");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYTCP_PORT);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("172.0.0.1");
	bzero(&(my_addr.sin_zero), 8);

    // Binding socket
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));
	if (ret < 0) {
		printf("error in binding");
		exit(1);
	}
	
    // Listen socket
	ret = listen(sockfd, BACKLOG);
	if (ret < 0) {
		printf("error in listening");
		exit(1);
	}

    while(1) {
        printf("Waiting for data\n");
        sin_size = sizeof(struct sockaddr_in);

        // Accepting connection
		con_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (con_fd < 0) {
			printf("Error in accept\n");
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
    char buf[BUFSIZE];
	FILE *fp;
	char recvs[DATALEN];
	struct ack_so ack;
	int end, n, count;
	long lseek=0;
	end = 0;
    n = 0;
    count = 0;
    // int sendCount = 0;

    printf("Receiving data!\n");

    while(!end) {
        // Receive packet
        if ((n = recv(sockfd, &recvs, DATALEN, 0)) == -1) {
			printf("error when receiving\n");
			exit(1);
		}

        // Checks if end of file
		if (recvs[n - 1] == '\0') {
			end = 1;
			n--;
		}
		memcpy((buf + lseek), recvs, n);
		lseek += n;
        count++;

        if (count == BATCH_SIZE || end == 1) {  // Sends ack after every batch
            count = 0;
            ack.num = BATCH_SIZE;
            ack.len = BATCH_SIZE;
            // sendCount++;
            // printf("Sending Ack %d!\n", sendCount);
            if ((n = send(sockfd, &ack, 2, 0)) == -1) {
                printf("Send ack error!");
                exit(1);
            }
        }
    }

    // Opening file to write to
    if ((fp = fopen ("myTCPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exist\n");
		exit(0);
	}
	fwrite (buf , 1 , lseek , fp);
	fclose(fp);
	printf("A file has been successfully received!\nThe total data received is %d bytes\n", (int)lseek);
}