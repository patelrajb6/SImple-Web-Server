//
//  main.c
//  server
//
//  Created by Raj Patel on 4/15/19.
// attribution: used the server.c from lab; manpage , beej's guide to socket programming
// I ran out of time so I just made two functions one as sequential and the other as concurrent
// they both have alot of things which is same which I ll fix later.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/uio.h>

void sigchld_handler(int s)                 //child reaper
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}


void concurrent(int port)
{
    int socketfd, newSocket,indexfd,filefd,x=0;            //server socket and incoming sockets
    char buffer[3000];                      // to get the data from index.html and read it into the buffer
    char address[1024];                     //buffer to store address
    struct sockaddr_in serverAddr;      //ipv4 structure to store addresses
    struct sockaddr_storage serverStorage;   //storing the incoming addresses
    socklen_t addr_size;
    signal(SIGCHLD,sigchld_handler);        //reaps the child after it finished the client's request
    
    
    
    /*---- Create the socket. The three arguments are: ----*/
    /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
    socketfd = socket(PF_INET, SOCK_STREAM, 0);
    /*---- Configure settings of the server address struct ----*/
    /* Address family = Internet */
    serverAddr.sin_family = AF_INET;
    /* Set port number, using htons function to use proper byte order */
    serverAddr.sin_port = htons(port);
    /* Set IP address to localhost */
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    /*---- Bind the address struct to the socket ----*/
    bind(socketfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    /*---- Listen on the socket, with 5 max connection requests queued ----*/
    if(listen(socketfd,5)==0)
        printf("Listening\n");
    else
        printf("Error\n");
    /*---- Accept call creates a new socket for the incoming connection ----*/
    addr_size = sizeof(serverStorage);
    filefd=open("log.txt",O_CREAT|O_RDWR|O_APPEND|O_TRUNC,0644);
    
    char *response ="HTTP/1.1 200 OK\r\n"        //html header
    "Content-Type: text/html; charset=UTF-8\r\n\r\n";
    
    char buf[2000];
    while (1)
    {
        
        newSocket = accept(socketfd, (struct sockaddr *) &serverStorage, &addr_size);
        //shutdown(newSocket, SHUT_RD);
        recv(newSocket, &buf, sizeof(buf), 0);
        x++;
        /*---- Send message to the socket of the incoming connection ----*/
        pid_t pid=fork();
        if(pid==0)
        {
            
            dup2(filefd,1 );
            close(socketfd);
            close(filefd);
            inet_ntop(AF_INET, &((struct sockaddr_in*)&serverStorage)->sin_addr, address, addr_size);
            printf("%d  %s %d\n",x,address,((struct sockaddr_in*)&serverStorage)->sin_port);
            
            send(newSocket,response,strlen(response),0);
            indexfd=open("index.html",O_RDWR,0644);
            while (1) {
                ssize_t p=read(indexfd,buffer,1024);
                if(p==0)
                    break;
                send(newSocket,buffer,p,0);
            }
            close(indexfd);
            close (newSocket);
            exit(0);
        }
        close (newSocket);
    }
}
void sequential(int port)
{
    int socketfd, newSocket,indexfd,filefd,x=0;            //server socket and incoming sockets
    char buffer[4000];                      // to get the data from index.html and read it into the buffer
    char address[1024];                     //buffer to store address
    struct sockaddr_in serverAddr;      //ipv4 structure to store addresses
    struct sockaddr_storage serverStorage;   //storing the incoming addresses
    socklen_t addr_size;
    signal(SIGCHLD,sigchld_handler);        //reaps the child after it finished the client's request
  
    
    
    /*---- Create the socket. The three arguments are: ----*/
    /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
    socketfd = socket(PF_INET, SOCK_STREAM, 0);
    /*---- Configure settings of the server address struct ----*/
    /* Address family = Internet */
    serverAddr.sin_family = AF_INET;
    /* Set port number, using htons function to use proper byte order */
    serverAddr.sin_port = htons(port);
    /* Set IP address to localhost */
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    /*---- Bind the address struct to the socket ----*/
    bind(socketfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    /*---- Listen on the socket, with 5 max connection requests queued ----*/
    if(listen(socketfd,5)==0)
        printf("Listening\n");
    else
        printf("Error\n");
    /*---- Accept call creates a new socket for the incoming connection ----*/
    addr_size = sizeof(serverStorage);
    filefd=open("log.txt",O_CREAT|O_RDWR|O_APPEND|O_TRUNC,0644);
    
    char *response ="HTTP/1.1 200 OK\r\n"        //html header
    "Content-Type: text/html; charset=UTF-8\r\n\r\n";
    
    char buf[5000];
    while (1)
    {
        newSocket = accept(socketfd, (struct sockaddr *) &serverStorage, &addr_size);
        x++;
        recv(newSocket, buf, sizeof(buf),0);
        /*---- Send message to the socket of the incoming connection ----*/
        dup2(filefd,1);
        inet_ntop(AF_INET, &((struct sockaddr_in*)&serverStorage)->sin_addr, address, addr_size);
        printf("%d  %s %d\n",x,address,((struct sockaddr_in*)&serverStorage)->sin_port);
        indexfd=open("index.html",O_RDWR,0644);
        
        send(newSocket,response,strlen(response),0);            //http header sent
        
        while (1) {
            ssize_t p=read(indexfd,buffer,1024);
            if(p==0)
                break;
            send(newSocket,buffer,p,0);
        }
        close(indexfd);
        close (newSocket);
    }
}

int main(int argc,char** argv){
    int port= atoi(argv[1]);
   
    //printf("%d\n",port);
#ifndef CONCURRENT
    sequential(port);
    sleep(1);
   
#endif
#ifdef CONCURRENT
    concurrent(port);
    sleep(1);
#endif
    return 0;
}
