/* 
 * File:   filecopy_rcv.c
 * Author: Ricardo Candussi
 * mail:   ricardo@metaebene.at
 *
 * Created on 03. April 2016, 01:34
 */

/*
 * filecopy_rcv
 * Usage: filecopy_snd -f <file> [-p <port>]
 */

#include <stdio.h>          //perror, 
#include <stdlib.h>

#include <arpa/inet.h>      //htons, htonl, ntohl, ... 
#include <fcntl.h>
#include <netdb.h>          //getprotobyname
#include <netinet/in.h>
#include <sys/stat.h>       //get filesize
#include <sys/socket.h>     //socket: bind, accept, recv, etc...
#include <unistd.h>         //read(), write(), getopt(3), etc...
#include <getopt.h>         //parse arguments argc argv
#include <string.h>

#include "defaultsettings_rcv.h"    //settings like std IP, port, etc.

int getoptions(int argc, char **argv, char** file_pathString, char** server_portString);
void help();

int main(int argc, char **argv) {
    
    char *file_path = NULL;
    char *server_port = NULL;
    char buffer[buffer_size];
    
    int client_sockfd;
    int enable = 1;                         //setsockopt
    int filefd;                             //fd file to  write
    int server_sockfd;                      //fd socket
    socklen_t client_len;
    ssize_t read_return;
    struct protoent *protoent;
    struct sockaddr_in client_address, server_address;
    int userver_port;
    
    u_int32_t file_size_len = 0;            //filesize recieved from client
    u_int32_t file_size_len_MB = 0;
    struct stat sb;                         //for file size
    u_int32_t file_size_ck = 0;             //actual size after recieving
    u_int32_t file_size_ck_MB = 0;
    
    u_int32_t rcvd = 0;
    u_int32_t rcvdMB = 0;
    
    memset(&buffer[0], 0, sizeof(buffer));  //set array to zero (init state))
    
    /*getopt*/
    getoptions(argc, argv, &file_path, &server_port);   //check input arguments and return at least 'file_path'
    
    userver_port = atoi(server_port);                   //convert char to int for 'sockaddr_in.sin_port = htons(...);'
    
    /* Create a socket and listen to it.. */
    protoent = getprotobyname(default_protocol);
    
    if (protoent == NULL) {
        perror("getprotobyname");
        exit(EXIT_FAILURE);
    }
    
    server_sockfd = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
    
    if (server_sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }
    
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(userver_port);
    
    if (bind(server_sockfd, (struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_sockfd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    fprintf(stderr, "listening on port %d\n", userver_port);
    
    /*receive from socket*/
    while (1) {
        client_len = sizeof(client_address);
        printf("waiting for client\n");

        client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_address, &client_len);
        printf("client connected...\n");
        
        filefd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);  //open file to store received data
        
        if (filefd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        /*receive file size*/
        read(client_sockfd, buffer, sizeof(buffer));        
        file_size_len = ntohl(atoi(buffer));                            //convert char to int for filesize
                                                                        //ntohl --> convert network to host byte order
        file_size_len_MB = file_size_len / MB_devider;
        printf("\nBytes expected:\t%u (%u MB)\n", file_size_len, file_size_len_MB);
        
        /*receive actual file*/
        do {
            
            read_return = read(client_sockfd, buffer, sizeof(buffer));  //receive file
            
            if (rcvd != file_size_len) {                                //print received bytes
            printf("Bytes received:\t%u (%u MB)\r", rcvd, rcvdMB);
            }
            
            if (read_return == -1) {
                perror("read");
                remove(file_path);                                       //delete file in case of failure
                exit(EXIT_FAILURE);
            }
         
            if (write(filefd, buffer, read_return) == -1) {
                perror("write");
                remove(file_path);                                      //delete file in case of failure
                exit(EXIT_FAILURE);
            }
            
            rcvd+=read_return;                                          //addition of receives bytes
            rcvdMB = rcvd/MB_devider;
            
        } while (read_return > 0);                                      //receive until 'read' returns 0 --> no more to read
        
        /*get size of received file*/
        if(stat (file_path, &sb)!=0){
            perror("stat (filesize)");
            exit (EXIT_FAILURE);
        }
        file_size_ck=sb.st_size;                                        //get local size of file after receiving
        file_size_ck_MB = file_size_ck / MB_devider;
        
        if (file_size_ck < file_size_len || file_size_ck > file_size_len) { //check if received filesize equals the expected filesize
            printf("\nError while receiving file!\n");
            printf("%u Bytes (%u MB) received. (%u Bytes (%u MB) expected)\n", file_size_ck, file_size_ck_MB, file_size_len, file_size_len_MB);
            remove(file_path);                                              //delete received (corrupted) file
            printf("corrupted data has been deleted...\n");
            close(filefd);
            close(client_sockfd);
            exit(EXIT_FAILURE);
        }
        printf("Bytes received:\t%u (%u MB)\n***file successfully received from client***\n\nEXIT\n",file_size_ck, file_size_ck_MB);
        close(filefd);
        close(client_sockfd);
        return EXIT_SUCCESS;    //close filecops_rcv after recieving a file
    } 
    return EXIT_SUCCESS;        //typically this piont should not be reached
}
/*getoptions with getopt*/
int getoptions(int argc, char **argv, char** file_pathString, char** server_portString) {
    
    int opt = 0;

    *file_pathString = NULL;
    *server_portString = NULL;
    
    while ((opt = getopt(argc, argv, "hf:p:")) != -1)
        switch (opt){
            case 'h':
                help();
                exit(EXIT_SUCCESS);
                break;
            case 'f':       //getopt file path *neccesary
                *file_pathString = optarg;
                printf ("filepath:\t<%s>\n", optarg);
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
        
    if (*server_portString == NULL)   //check if option '-p' is specified
    {                                 //if not -> set default values
        *server_portString = default_server_Port;
        printf("Using default port:\t'%s'\n", default_server_Port); 
    }
        
    return(0);
}
void help(){
    
    printf("\n********************************************\n");
    printf("**************filecopy_rcv V1***************\n");
    printf("********************************************\n");
    printf("\nUsage:\n");
    printf("\tfilecopy_snd -f <file> [-p <port>]\n\n");
    printf("\nOptions:\n");
    printf("\tThe following options are available:\n\n");
    printf("\t-h\tdisplay this page.\n\n");
    printf("\t-p\tspecify the Port to be used for establishing a connection.\n\t\te.g.: <1234> (if unused tha default setting will be loaded as specified in 'defaultsettings.h'\n\n\n");
}
