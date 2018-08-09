
//Compile using: g++ -o myServer server.c -w
//-w flag to remove warnings
// Run using ./myServer 9876 
//Client has specified this port, so if you change it when you run, change it in the client too.


//for reference: http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html


#include <stdio.h>
#include <sys/types.h> 
// This header file contains definitions of a 
// number of data types used in system calls. 
// These types are used in the next two include files.

#include <sys/socket.h>
// includes a number of definitions of structures needed for sockets.
#include <netinet/in.h>
//contains constants and structures needed for internet domain addresses.
#include <netdb.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 


void dostuff(int); /* function prototype */

void error(char *msg)
{
    perror(msg);
    exit(1);
}
//called when a system call fails. 
// It displays a message about the error on stderr 
// and then aborts the program

char buffer[256];

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, pid;
     socklen_t clilen;

// -sockfd and newsockfd are file descriptors.
//    These two variables store the values returned by the socket system call 
//    and the accept system call.
// -portno stores the port number on which the server accepts connections.
// -clilen stores the size of the address of the client. 
//    This is needed for the accept system call.
// -n is the return value for the read() and write() calls; 
    // i.e. it contains the number of characters read or written.

     struct sockaddr_in serv_addr, cli_addr;
     //A sockaddr_in is a structure containing an internet address. 
     //This structure is defined in <netinet/in.h>. Here is the definition:
      // struct sockaddr_in {
      //         short   sin_family;
      //         u_short sin_port;
      //         struct  in_addr sin_addr;
      //         char    sin_zero[8];
      // };
     //An in_addr structure, defined in the same header file, 
     //contains only one field, a unsigned long called s_addr.
     //The variable serv_addr will contain the address of the 
     //server, and cli_addr will contain the address of the client 
     //which connects to the server.

     if (argc < 2) { //The user needs to pass in the port number on 
      // which the server will accept connections as an argument. 
      // This code displays an error message if the user fails to do this.
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

////
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
//The socket() system call creates a new socket. 
//It takes three arguments:
// 1. Address domain of the socket.
// 2. Type of socket of socket
// 3. Protocol
  //  If this argument is zero (and it always should be except for 
//    unusual circumstances), the operating system will choose the most 
//    appropriate protocol
//Socket system call returns a small integer. 
//This value is used for all subsequent references to this socket. 
//If the socket call fails, it returns -1. 
////

////
     bzero((char *) &serv_addr, sizeof(serv_addr)); //sets all values in buffer to zero
     // It takes two arguments: 
    //1. Pointer to the buffer
    //2. Size of the buffer. 
    //Thus, this line initializes serv_addr to zeros.
////     
     portno = atoi(argv[1]); //port number on which server will listen 
     //for connections needs to be passed in!

     serv_addr.sin_family = AF_INET;//first field of serv_adder- should always be set to the symbolic constant AF_INET.
     
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     //third field of sockaddr_in is a structure of type struct 
     // in_addr which contains only a single field unsigned long s_addr. 
     // This field contains the IP address of the host. For server code, 
     // this will always be the IP address of the machine on which the 
     // server is running, and there is a symbolic constant INADDR_ANY which 
     // gets this address.


     serv_addr.sin_port = htons(portno);
    //second field of serv_addr is unsigned short sin_port , 
     //which contains the port number. However, instead of simply
     //copying the port number to this field, it is necessary to 
     //convert this to network byte order using the function htons() 
     //which converts a port number in host byte order to a port number 
     //in network byte order.

     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
//The bind() system call binds a socket to an address, 
// in this case the address of the current host and port number 
// on which the server will run. 
// It takes three arguments:
// 1. Socket file descriptor, the address to which is bound, and 
// the size of the address to which it is bound. 
// 2.Pointer to a structure of type sockaddr, 
// but what is passed in is a structure of type sockaddr_in, 
// and so this must be cast to the correct type. 
// 3. Size of buffer



     listen(sockfd,5);//listen system call allows the process to 
     //listen on the socket for connections
     
     clilen = sizeof(cli_addr);
     while (1) {
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0) 
             error("ERROR on accept");
         pid = fork();
         if (pid < 0)
             error("ERROR on fork");
         if (pid == 0)  {
             close(sockfd);
             dostuff(newsockfd);
             exit(0);
         }
         else close(newsockfd);
     } /* end of while */
     return 0; /* we never get here */
}

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void dostuff (int sock)
{
   int n;
   //server reads characters from the socket connection into this buffer.
      
   bzero(buffer,256);
   n = read(sock,buffer,255);
   if (n < 0) error("ERROR reading from socket");
   printf("%s\n" buffer);
   n = write(sock,"I got your message",18);
   if (n < 0) error("ERROR writing to socket");
}