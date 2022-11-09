#include "headsock.h"

#define BATCH_SIZE 1

float str_cli(FILE *fp, int sockfd, long *len);  //transmit function
void tv_sub(struct  timeval *out, struct timeval *in);  //calculate time

int main(int argc, char **argv) {
    FILE *fp;
    struct sockaddr_in ser_addr;
    struct hostent *sh;
    struct in_addr **addrs;

    int sockfd, ret;
    float timeInterval, dataRate;
    long len;
    char ** pptr;

    if (argc != 2) {
        printf("Parameters not match!\n");
    }

    // Get host's information
    sh = gethostbyname(argv[1]);
    if (sh == NULL) {
        printf("Error when gethostby name!\n");
        exit(0);
    }

    // Print the remote host's information
    // printf("canonical name: %s\n", sh->h_name);
    for (pptr=sh->h_aliases; *pptr != NULL; pptr++) {
        printf("the aliases name is: %s\n", *pptr);
    }
    // switch(sh->h_addrtype) {
    // 	case AF_INET:
    // 		printf("AF_INET\n");
    // 	break;
    // 	default:
    // 		printf("unknown addrtype\n");
    // 	break;
    // }
        
    addrs = (struct in_addr **)sh->h_addr_list;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);  // Create socket
    if (sockfd < 0) {
        printf("Error in creating socket!\n");
        exit(1);
    }
    ser_addr.sin_family = AF_INET;                                                      
    ser_addr.sin_port = htons(MYTCP_PORT);
    memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
    bzero(&(ser_addr.sin_zero), 8);
    ret = connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr));         //connect the socket with the host
    if (ret != 0) {
        printf ("Connection failed!\n"); 
        close(sockfd); 
        exit(1);
    }

    if((fp = fopen("myfile.txt", "r+t")) == NULL) {
        printf("File doesn't exit\n");
        exit(0);
    }

    timeInterval = str_cli(fp, sockfd, &len);  //performs transmitting and receiving: returns the total time taken.
    dataRate = (len/(float)timeInterval);
    printf("%f\n", timeInterval);
    printf("%f\n", dataRate);
    // printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\n", timeInterval, (int)len, dataRate);

    close(sockfd);
    fclose(fp);
    exit(0);
}

float str_cli(FILE *fp, int sockfd, long *len) {
    char *buff;
    long fileSize, currIndex;
	char sends[DATALEN];
	struct pack_so packet;
	struct ack_so ack;
	struct timeval sendTime, recvTime;

	int slen, n;
	float timeInterval = 0.0;
	currIndex = 0;

    // Get the size of the file
    fseek(fp, 0, SEEK_END);
    fileSize = ftell (fp);
	rewind (fp);
	// printf("The file length is %d bytes\n", (int)fileSize);
	// printf("the packet length is %d bytes\n",DATALEN);

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
        for (int i = 0; i < BATCH_SIZE; i++) {  // Sends n packets in one batch
            if ((fileSize + 1 - currIndex) <= DATALEN) {
                slen = fileSize + 1 - currIndex;  // Gets len of data left if less than packet size
            } else {
                slen = DATALEN;
            }
            memcpy(packet.data, (buff + currIndex), slen);
			packet.num = i + 1;
			packet.len = slen;

			// Sends the packet
            n = send(sockfd, &packet, PACKLEN, 0);
            if (n == -1) {
                printf("Send error!\n");
                exit(1);
            }
            currIndex += slen;
        }
        
        // Receives ack
        if ((n = recv(sockfd, &ack, 2, 0)) == -1) {
		printf("Error when receiving!\n");
		exit(1);
	    }

        // Checks ack
        if (ack.num != 1 || ack.len != 1) {
            printf("Error in transmission!\n");
        }
    }
    gettimeofday(&recvTime, NULL);
    *len = currIndex;
    tv_sub(&recvTime, &sendTime);
    timeInterval += (recvTime.tv_sec) * 1000.0 + (recvTime.tv_usec) / 1000.0;
    return(timeInterval);
}

void tv_sub(struct timeval *out, struct timeval *in) {
    if ((out->tv_usec -= in->tv_usec) < 0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}