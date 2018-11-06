
// UDP client program
# include <stdio.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <string.h>
# include <stdlib.h>
# include <arpa/inet.h>

#define PORT 2020
#define MAXLINE 1024
int main()
{
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;

    int n;
    socklen_t len;
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket creation failed");
        exit(0);
    }
    printf("Socket() done..\n");
    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // send hello message to server
    printf("Sending..." );
    sendto(sockfd, "+", strlen("+"),
           0, (const struct sockaddr*)&servaddr,
           sizeof(servaddr));
    printf("Done\n");
    // receive server's response
    printf("Waiting for response...\n");
    n = recvfrom(sockfd, (char*)buffer, MAXLINE, 0, (struct sockaddr*)&servaddr,&len);
    if(n<=0){
        perror("recvfrom()");
        close(sockfd);
        exit(1);
    }
    puts(buffer);
    close(sockfd);
    return 0;
}
