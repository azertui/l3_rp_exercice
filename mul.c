
// UDP client program
# include <stdio.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <string.h>
# include <stdlib.h>
# include <arpa/inet.h>
# include <errno.h>
# include <time.h>

#define PORT 2020
#define MAXLINE 1024


int main()
{
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;
    int calcul=0;
    int n;
    socklen_t len;
    time_t t;
    srand((unsigned) time(&t));
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

    struct timeval read_timeout;
    read_timeout.tv_sec = 10;
    read_timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

    for (;;) {
    // send hello message to server
    printf("Alive...\n" );
    sendto(sockfd, "*", strlen("*"),
           0, (const struct sockaddr*)&servaddr,
           sizeof(servaddr));
    //printf("Done\n");
    // receive server's response
    printf("Waiting for response...\n");
    n = recvfrom(sockfd, (char*)buffer, MAXLINE, 0, (struct sockaddr*)&servaddr,&len);
    if(n<=0 && errno != EAGAIN){
        perror("recvfrom()");
        close(sockfd);
        exit(1);
    }
    else if(n>0)
      calcul=1;
    //calcul
    if(calcul){
      char resS[MAXLINE];
        int i=2;
        int a = 0,b = 0,res = 0,erreur=0,moins=0;
        if(buffer[i-1]!='(')
          erreur=1;
        if(buffer[i]=='-'){
          moins=1;
          i++;
        }
        if(buffer[i]=='+')
          i++;
        while(buffer[i]!=',' && i<strlen(buffer)){
          a *= 10;
          a += buffer[i] - '0';
          i++;
        }
      i++;
      if(moins)
        a = 0-a;
      moins = 0;
      if(buffer[i]=='-'){
        moins=1;
        i++;
      }

      if(buffer[i]=='+')
        i++;
      while(buffer[i]!=')' && i<strlen(buffer)){
        b *= 10;
        b += buffer[i] - '0';
        i++;
      }
      if(moins)
        b = 0-b;
      if(i>=strlen(buffer))
        erreur=1;

      if (erreur==1){
        resS[0] = 'E';
        resS[1] = 'R';
        resS[2] = 'R';
        resS[3] = 'E';
        resS[4] = 'U';
        resS[5] = 'R';
        resS[6] = '\0';}
      else{
        res = a*b;
        snprintf(resS,MAXLINE,"%d",res);
      }
      usleep((rand()%16)*1000000);
      printf("%s\n",resS );
      printf("Sending..." );
      sendto(sockfd, resS, MAXLINE,
             0, (const struct sockaddr*)&servaddr,
             sizeof(servaddr));
      printf("Done\n");
      calcul=0;
    }
}
    close(sockfd);
    return 0;
}
