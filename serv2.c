#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#define PORT 2020
#define MAXLINE 1024

struct noeud
{
  struct in6_addr addr;
  int port;
  char op;
  double temps;
};
//On retire les noeuds n'ayant pas envoyé de messages depuis plus de 35s
void verification(struct noeud *nodes, int *nbnode){
  int i,j;
  double temps=time(NULL);
  printf("\n");
  printf("%d noeuds\n",*nbnode);
  for(i=0;i<*nbnode;i++){
    printf("dt=%lf\n",(double)temps-nodes[i].temps);
    if(temps-nodes[i].temps>35){
      for(j=i;j<(*nbnode)-1;j++){
        nodes[j]=nodes[j+1];
      }
      (*nbnode)--;
    }
  }
}

int main()
{
		int nbnode=0;
		int nnodemax=5;
		struct noeud *nodes=(struct noeud*)malloc(nnodemax*sizeof(struct noeud));

    int udpfd, fd1, fd_send;
    char buffer[MAXLINE];
    fd_set rset;
    ssize_t n;
    socklen_t len;
    struct sockaddr_in6 cliaddr, servaddr,s_send;
    char* message = "+(10,2)";
    void sig_chld(int);



    /* create UDP socket */
    if((udpfd = socket(AF_INET6, SOCK_DGRAM, 0))==-1)
		{
			perror("Server UDP socket()");
			exit(1);
		}
    if((fd_send = socket(AF_INET6, SOCK_DGRAM, 0))==-1)
		{
			perror("Server UDP socket()");
			exit(1);
		}
    //create server ipv6 addr
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_addr = in6addr_any;
    servaddr.sin6_port = htons(PORT);
    // binding server addr structure to udp sockfd
    if((bind(udpfd, (struct sockaddr*)&servaddr, sizeof(servaddr)))==-1)
		{
			perror("Server UDP bind()");
			exit(1);
		}
    // clear the descriptor set
    FD_ZERO(&rset);

    fd1 = udpfd + 1;
		printf("Entering loop..\n");
    for (;;) {
				printf("Waiting..\n");
        // set udpfd in readset
        FD_SET(udpfd, &rset);
        FD_SET(0, &rset); //add standard input
        // select the ready descriptor
        if(select(fd1, &rset, NULL, NULL, NULL)==-1)
				{
					perror("Server-select()");
					exit(1);
				}
        verification(nodes,&nbnode);
        // if udp socket is readable
        if (FD_ISSET(udpfd, &rset)) {
            len = sizeof(cliaddr);
            bzero(buffer, sizeof(buffer));
            printf("Message from UDP client: ");
            n = recvfrom(udpfd, buffer, sizeof(buffer), 0,(struct sockaddr*)&cliaddr, &len);
						if(n<=0){
								perror("recvfrom()");
								close(udpfd);
								exit(1);
						}
						puts(buffer);
						if(nbnode==nnodemax){
							nnodemax+=5;
							nodes= (struct noeud*) realloc(nodes,nnodemax*sizeof(struct noeud));
							if(nodes==NULL){
								perror("realloc");
								exit(1);
							}
						}
						nodes[nbnode].addr=cliaddr.sin6_addr;
						nodes[nbnode].port=cliaddr.sin6_port;
            nodes[nbnode].op=buffer[0];
            nodes[nbnode].temps=time(NULL);
						nbnode++;
							printf("New %c node added",buffer[0]);
						printf("Client port: %d\n",cliaddr.sin6_port);
						/*printf("Sending response..");
            sendto(udpfd, (const char*)message, sizeof(buffer), 0,
                   (struct sockaddr*)&cliaddr, sizeof(cliaddr));
						printf("Done\n");*/
        }
        // if standard input is readable
        if (FD_ISSET(0, &rset)) {
          printf("Message terminal...");
          read(0, buffer, sizeof(buffer));
          printf("%s\n",buffer);
          //envoit du message au noeud (Un seul conidéré à changer , ne vérifie pas la syntaxe)
          int tmp=0;
          while (nodes[tmp].op != buffer[0] && tmp < nbnode){
            tmp++;
          }
          if(tmp < nbnode){

            bzero(&s_send, sizeof(s_send));
              cliaddr.sin6_family = AF_INET6;
              cliaddr.sin6_addr = nodes[tmp].addr;
              cliaddr.sin6_port = nodes[tmp].port;


          printf("Sending response..");
          sendto(udpfd, (const char*)buffer, sizeof(buffer), 0,
                 (struct sockaddr*)&cliaddr, sizeof(cliaddr));
          printf("Done\n");
          buffer[0]='\0';
          n = recvfrom(udpfd, buffer, sizeof(buffer), 0,(struct sockaddr*)&cliaddr, &len);
          printf("%s\n","Résultat : " );
          puts(buffer);
        }
        else{
          printf("Aucun noeud disponible\n");
        }
        buffer[0]='\0';
    }
  }
}
