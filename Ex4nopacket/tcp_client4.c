#include "headsock.h"

#define BATCH_SIZE 2

float str_cli(FILE *fp, int sockfd, long *len);  //transmit function
void tv_sub(struct  timeval *out, struct timeval *in);  //calculate time

int main(int argc, char **argv) {
	int sockfd, ret;
	float ti, rt;
	long len;
	struct sockaddr_in ser_addr;
	char ** pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	FILE *fp;

	if (argc != 2) {
		printf("parameters not match");
	}

	sh = gethostbyname(argv[1]);	                                       //get host's information
	if (sh == NULL) {
		printf("error when gethostby name");
		exit(0);
	}

	printf("canonical name: %s\n", sh->h_name);					//print the remote host's information
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++) {
		printf("the aliases name is: %s\n", *pptr);
    }
	switch(sh->h_addrtype) {
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}
        
	addrs = (struct in_addr **)sh->h_addr_list;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);                           //create the socket
	if (sockfd <0) {
		printf("error in socket");
		exit(1);
	}
	ser_addr.sin_family = AF_INET;                                                      
	ser_addr.sin_port = htons(MYTCP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);
	ret = connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr));         //connect the socket with the host
	if (ret != 0) {
		printf ("connection failed\n"); 
		close(sockfd); 
		exit(1);
	}
	
	if((fp = fopen ("myfile.txt", "r+t")) == NULL) {
		printf("File doesn't exit\n");
		exit(0);
	}

    ti = str_cli(fp, sockfd, &len);  //performs transmitting and receiving; returns the total time taken.
    rt = (len/(float)ti);
	printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\n", ti, (int)len, rt);

    close(sockfd);
    fclose(fp);
    exit(0);
}

float str_cli(FILE *fp, int sockfd, long *len) {
    char *buff;
    long fileSize, currIndex;
	char sends[DATALEN];
	struct ack_so ack;
	int n, slen, status;
	float timeInterval = 0.0;
	struct timeval sendTime, recvTime;
	currIndex = 0;
    n = BATCH_SIZE;
    // int recvCount = 0;

    // Get the size of the file
    fseek(fp, 0, SEEK_END);
    fileSize = ftell (fp);
	rewind (fp);
	printf("The file length is %d bytes\n", (int)fileSize);
	printf("the packet length is %d bytes\n",DATALEN);

    // Allocate memory for file
    buff = (char *) malloc (fileSize);
    if (buff == NULL) {
        exit(2);
    }

    // Copying file into buffer
    fread(buff, 1, fileSize, fp);

    // Append end byte
    buff[fileSize] = '\0';

    gettimeofday(&sendTime, NULL);

    while (currIndex <= fileSize) {
        for (int i = 0; i < n; i++) {  // Sends n packets in one batch
            if ((fileSize + 1 - currIndex) <= DATALEN) {
                slen = fileSize + 1 - currIndex;  // Gets len of data left if less than packet size
            } else {
                slen = DATALEN;
            }
            memcpy(sends, (buff+currIndex), slen);

            status = send(sockfd, &sends, slen, 0);
            if (status == -1) { // Sends the packet
                printf("send error!");
                exit(1);
            }
            currIndex += slen;
        }
        
        // Receives ack
        if ((status = recv(sockfd, &ack, 2, 0)) == -1) {
		printf("error when receiving\n");
		exit(1);
	    }

        // recvCount++;
        // printf("Received %d Acks\n", recvCount);

        // Checks ack
        if (ack.num != n || ack.len != n) {
            printf("error in transmission\n");
        }
    }
    gettimeofday(&recvTime, NULL);
    *len = currIndex;
    tv_sub(&recvTime, &sendTime);
    timeInterval += (recvTime.tv_sec)*1000.0 + (recvTime.tv_usec)/1000.0;
    return(timeInterval);
}

void tv_sub(struct timeval *out, struct timeval *in) {
    if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}