/* 
 * File:   filecopy_snd.c
 * Author: Ricardo Candussi
 * mail:   ricardo@metaebene.at
 *
 * Created on 03. April 2016, 01:29
 */

/*
 * filecopy_snd
 * Usage: filecopy_snd -f <file> [-i <ip>] [-p <port>]
 */

#include <stdio.h>          //perror,
#include <stdlib.h>

#include <arpa/inet.h>      //htons, htonl, ntohl, ... 
#include <fcntl.h>          //open() read() write() close() file
#include <netdb.h>          //used for hostname resolve
#include <netinet/in.h>
#include <sys/stat.h>       //get filesize
#include <sys/types.h>
#include <sys/socket.h>     //socket: bind, accept, recv, etc...
#include <unistd.h>         //read(), write(), getopt(3), etc...
#include <string.h>         //memset
#include <getopt.h>         //parse arguments argc argv

#include "defaultsettings_snd.h"

int getoptions(int argc, char **argv, char** file_pathString, char** server_hostString, char** server_portString);
void help();

int main(int argc, char **argv) {

    struct protoent *protoent;
    char *file_path = NULL;                    
    char *server_host = NULL;  
    char *server_port = NULL;
    char buffer[buffer_size];      
    
    in_addr_t in_addr;                      //<arpa/inet.h>
    int filefd;                             //fd for file to send
    int sockfd = 0;                         //fd socket
    ssize_t read_return;
    struct hostent *hostent;                //for hostname
    struct sockaddr_in sockaddr_in;
    int userver_port = 0;                   //used server port from 'getopt'
    
    struct stat sb;                         //for file size --> stat
    u_int32_t file_size = 0;
    u_int32_t file_size_MB = 0;
    char file_size_send[1024];              //"buffer" for filesize send to server
    ssize_t file_len = 0;
    
    u_int32_t sndd = 0;
    u_int32_t snddMB = 0;
    
    memset(&file_size_send[0], 0, sizeof(file_size_send));      //set array to zero (init state))
    memset(&buffer[0], 0, sizeof(buffer));                      //set array to zero (init state))
         
    /*getopt*/
    getoptions(argc, argv, &file_path, &server_host, &server_port); //check input arguments and return at least 'file_path'
    
    userver_port = atoi(server_port);                               //convert char to int for 'sockaddr_in.sin_port = htons(...);'

    /*get size of file*/
    if(stat (file_path, &sb) == 0){
        if(stat (file_path, &sb) != 0) {
            perror("stat");
            exit (EXIT_FAILURE);
        }
        
        if (sb.st_mode & __S_IFDIR ) {                 //check if really a file -> if is directory -> EXIT
            printf("File: is a directory!\n");
            exit(EXIT_FAILURE);
        }
    }
    
    file_size = (sb.st_size);
    
    if(file_size>max_file_size){                   //check actual file size to max size allowed
        printf("File to big. Max 4GB supported!\n");
        exit (EXIT_FAILURE);
    }
    
    file_size_MB = (file_size / MB_devider);        //convert Bytes to MB
    printf("Bytes to send:\t%u (%u MB)\n", file_size, file_size_MB);
    sprintf(file_size_send, "%u", htonl(file_size));//file_size_send conversion int to char for sending over socket
                                                    //htonl -> convert host to network byte order
    
    /*open file*/
    filefd = open(file_path, O_RDONLY);             //open file in readonly mode
    
    if (filefd == -1) {
        perror("open File");
        exit(EXIT_FAILURE);
    }
    
    /*get socket*/
    protoent = getprotobyname(default_protocol); //get the used protocol by resolving "char=tcp" with 'getprotobyname'-function of *netdb.h*
    
    if (protoent == NULL) {                      //print error and exit if protocol could not be resolved
        perror("getProtocolERROR");
        exit(EXIT_FAILURE);
    }
    
    sockfd = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
    
    if (sockfd == -1) {                         //print error and exit if socket could not be created
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    /*prepare sockaddr_in*/
    hostent = gethostbyname(server_host);       //resolve hostname
    
    if (hostent == NULL) {
        fprintf(stderr, "error: gethostbyname(\"%s\")\n", server_host);
        exit(EXIT_FAILURE);
    }
    
    in_addr = inet_addr(inet_ntoa(*(struct in_addr*)*(hostent->h_addr_list)));
    
    if (in_addr == (in_addr_t)-1) {
        fprintf(stderr, "error: inet_addr(\"%s\")\n", *(hostent->h_addr_list));
        exit(EXIT_FAILURE);
    }
    
    sockaddr_in.sin_addr.s_addr = in_addr;
    sockaddr_in.sin_family = AF_INET;
    sockaddr_in.sin_port = htons(userver_port);
    
    /*connect socket*/
    if (connect(sockfd, (struct sockaddr*) &sockaddr_in, sizeof(sockaddr_in)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    
    /*send the file size*/
    file_len = send(sockfd, file_size_send, sizeof(file_size_send), 0);     //send the file size to compare expect vs. received bytes
    if (file_len == -1) {
    	perror("file_len");
    	exit(EXIT_FAILURE);
    }

    /*send the file*/
    while (1) {
        
        read_return = read(filefd, buffer, sizeof(buffer));
        
        if (read_return == 0) {                             //stay in while until 'read_return' is 0
            printf("Bytes sent:\t%u (%u MB)", file_size, file_size_MB);
            printf("\n***File successfully sent to server***\n\nEXIT\n");
            break;
        }
        
        if (read_return == -1) {                            //error handling of file read 
            perror("read");                                 //close file
            close(filefd);
            exit(EXIT_FAILURE);
        }
        
        if (write(sockfd, buffer, read_return) == -1) {     //actual sending of file with error handling
            perror("write");                                //close file
            close(filefd);
            exit(EXIT_FAILURE);
        }
        
        sndd+=read_return;                                  //addition of receives bytes
        snddMB = sndd/MB_devider;
        
        printf("Bytes sent:\t%u (%u MB)\r", sndd, snddMB);  //print sent bytes
    } 
    
    close(filefd);                                          //close file after successfull transmission
    exit(EXIT_SUCCESS);                                     //exit filecopy_snd 
}
/*getoptions with getopt*/
int getoptions(int argc, char **argv, char** file_pathString, char** server_hostString, char** server_portString) {
    
    int opt = 0;
    
    *file_pathString = NULL;
    *server_hostString = NULL;
    *server_portString = NULL;
  
    while ((opt = getopt(argc, argv, "hf:i:p:")) != -1)
        switch (opt){
            case 'h':
                help();
                exit(EXIT_SUCCESS);
                break;
            case 'f':       //getopt file path *neccesary
                *file_pathString = optarg;
                printf ("filepath:\t<%s>\n", optarg);
                break;
            case 'i':       //getopt IP *optional
                *server_hostString = optarg;
                printf("IP used:\t\"%s\"\n", optarg);
                break;
            case 'p':       //getopt port *optional
                *server_portString = optarg;
                printf("Port used:\t\"%s\"\n", optarg);
                break;
            default:
                help();
                exit(EXIT_FAILURE);
        }
       
    if (*file_pathString == NULL)   //check if a file path is specified
    {
        printf("\nUsage: filecopy_snd -f <file> [-i <ip>] [-p <port>]\n\n"); 
        exit(EXIT_FAILURE);
    }
        
    if (*server_hostString == NULL)   //check if option '-i' is specified
    {                                 //if not -> set default values
        *server_hostString = default_server_IP;
        printf("Using default IP:\t'%s'\n", default_server_IP); 
    }
        
    if (*server_portString == NULL)   //check if option '-p' is specified
    {                                 //if not -> set default values
        *server_portString = default_server_Port;
        printf("Using default port:\t'%s'\n", default_server_Port);
    }
        
    return(0);
}
void help(){                            //print help (man)
    printf("\n********************************************\n");
    printf("**************filecopy_snd V1***************\n");
    printf("********************************************\n");
    printf("\nUsage:\n");
    printf("\tfilecopy_snd -f <file> [-i <ip>] [-p <port>]\n\n");
    printf("\nOptions:\n");
    printf("\tThe following options are available:\n\n");
    printf("\t-h\tdisplay this page.\n\n");
    printf("\t-i\tspecify the IP-adress /hostname of the target system.\n\t\te.g.: <192.168.10.117> or <yourhost.local> (if unused tha default setting will be loaded as specified in 'defaultsettings.h'\n\n");
    printf("\t-p\tspecify the Port to be used for establishing a connection.\n\t\te.g.: <1234> (if unused tha default setting will be loaded as specified in 'defaultsettings.h'\n\n\n");
}
