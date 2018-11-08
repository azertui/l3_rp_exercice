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

int udpfd;
struct sockaddr_in6 cliaddr;
struct noeud
{
  struct in6_addr addr;
  int port;
  char op;
  double temps;
  char* calcul;
  int hascalc;
};

//On regarde si le noeud est déjà connu
int existence(struct noeud *nodes, int nbnode,struct in6_addr addr, int port){
  int i;
  for(i=0;i<nbnode;i++){
    if(nodes[i].port ==port){
      nodes[i].temps=time(NULL);
      return 1;
    }
  }
  return 0;
}
//renvoie l'index d'un noeud dont l'existence a déjà été vérifiée
int find(struct noeud *nodes, int nbnode, int port){
  int i;
  for(i=0;i<nbnode;i++){
    if(nodes[i].port ==port){
      break;
    }
  }
  return i;
}
//On retire les noeuds n'ayant pas envoyé de messages depuis plus de 35s
void verification(struct noeud *nodes, int *nbnode){
  int i,j,transmit=0;
  char calcul[MAXLINE];
  double temps=time(NULL);
  for(i=0;i<*nbnode;i++){
    printf("%c use=%d\n",nodes[i].op,nodes[i].hascalc );
    //si pas de réponse depuis 35s
    if(temps-nodes[i].temps>35){
      if(nodes[i].hascalc){
        strncpy(calcul,nodes[i].calcul,strlen(nodes[i].calcul));
        transmit=1;
      }
      free(nodes[i].calcul);
      for(j=i;j<(*nbnode)-1;j++){
        nodes[j]=nodes[j+1];
      }
      (*nbnode)--;
    }
    if(transmit){
      for(i=0;i<*nbnode && transmit;i++){
        if(nodes[i].op == calcul[0]){
          cliaddr.sin6_family = AF_INET6;
          cliaddr.sin6_addr = nodes[i].addr;
          cliaddr.sin6_port = nodes[i].port;
          nodes[i].hascalc=1;
          strncpy(nodes[i].calcul,calcul,strlen(calcul));
          printf("node died, transmitting its calcul %s\n",calcul);
          sendto(udpfd, (const char*)calcul, sizeof(calcul), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
          transmit=0;
        }
      }
      if(transmit)
        printf("vérif: Aucun noeud disponible pour retransmettre l'opération %s\n", calcul);
    }
  }
  printf("\n");
  printf("%d noeuds\n",*nbnode);
}

int main()
{
		int nbnode=0;
		int nnodemax=5;
		struct noeud *nodes=(struct noeud*)malloc(nnodemax*sizeof(struct noeud));

    int fd1, fd_send, index;
    char buffer[MAXLINE];
    fd_set rset;
    ssize_t n;
    socklen_t len;
    struct sockaddr_in6 servaddr;



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
            //printf("Message from UDP client: ");
            n = recvfrom(udpfd, buffer, sizeof(buffer), 0,
            (struct sockaddr*)&cliaddr, &len);
						if(n<=0){
								perror("recvfrom()");
								close(udpfd);
								exit(1);
						}
						if(nbnode==nnodemax){
							nnodemax+=5;
							nodes= (struct noeud*) realloc(nodes,
              nnodemax*sizeof(struct noeud));
							if(nodes==NULL){
								perror("realloc");
								exit(1);
							}
						}
            if(!existence(nodes,nbnode,cliaddr.sin6_addr,cliaddr.sin6_port)){
  						nodes[nbnode].addr=cliaddr.sin6_addr;
  						nodes[nbnode].port=cliaddr.sin6_port;
              nodes[nbnode].op=buffer[0];
              nodes[nbnode].temps=time(NULL);
              nodes[nbnode].hascalc=0;
              nodes[nbnode].calcul=(char*)malloc(MAXLINE);
  						nbnode++;
            }

            else{
              index=find(nodes,nbnode,cliaddr.sin6_port);
              if(nodes[index].hascalc){
                if(buffer[0]!=nodes[index].op){
                  printf("noeud %d: %s <- %s",nodes[index].port,buffer,nodes[index].calcul);
                  nodes[index].hascalc=0;
                }
              }
            }
        }
        // if standard input is readable
        if (FD_ISSET(0, &rset)) {
          read(0, buffer, sizeof(buffer));
          //envoit du message au noeud (Un seul conidéré à changer , ne vérifie pas la syntaxe)
          int tmp=0;
          while ((nodes[tmp].hascalc || nodes[tmp].op != buffer[0]) && tmp < nbnode){
            printf("\nboucle%d\n",tmp);
            tmp++;
          }

          if(tmp < nbnode && !(nodes[tmp].hascalc) && nodes[tmp].op == buffer[0]){
            cliaddr.sin6_family = AF_INET6;
            cliaddr.sin6_addr = nodes[tmp].addr;
            cliaddr.sin6_port = nodes[tmp].port;
            strncpy(nodes[tmp].calcul,buffer,strlen(buffer));
            nodes[tmp].hascalc=1;
            sendto(udpfd, (const char*)buffer, sizeof(buffer), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
            memset(buffer,'\0',sizeof(buffer));
        }
        else{
          printf("Aucun noeud disponible\n");
        }
        memset(buffer,'\0',sizeof(buffer));
    }
  }
  free(nodes);
}
