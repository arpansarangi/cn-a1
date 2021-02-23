/* f20180195@hyderabad.bits-pilani.ac.in Arpan Sarangi */

/* Brief description of program...*/
/* ... */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

// ./http_proxy_download.out info.in2p3.fr 182.75.45.22 13128 csf303 csf303 index.html logo.gif
//             0                   1             2        3      4      5       6          7               

int main(int argc, char *argv[]) {
    if(argc < 7) {
        printf("\n Error : Arguments missing \n");
        return 1;
    } 

    int socket_desc;
    struct sockaddr_in serv_addr;

    //Determine if image needs to be found.
    int image = 0;
    if (strstr(argv[1], "info.in2p3.fr") != NULL) {
        image = 1;
    }
    
    if((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    //Get ip address of host.
    struct hostent *he;
    char host_name[100];
    memset(host_name, '0', sizeof(host_name));
    strcpy(host_name, argv[1]);
    if((he = gethostbyname(host_name)) == NULL) {
        printf("\n Error : Could not get ip of host \n");
        return 1;
    }
    
    // serv_addr.sin_addr.s_addr = inet_addr(ip);
    memcpy(&serv_addr.sin_addr, he -> h_addr_list[0], he -> h_length);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);


    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&serv_addr , sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Could not connect to server \n");
        return 1;
    }
    printf("Connected\n");
    

    char request[2000];
    char server_reply[100000];
    memset(request, '0', sizeof(request));
    // memset(server_reply, '0', sizeof(server_reply));
    sprintf(request, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host_name);
    if(send(socket_desc, request, strlen(request), 0) < 0)
    {
        printf("\n Error : Could not send request to server \n");
        return 1;
    }
    else printf("Data Send\n");
    
    //Receive a reply from the server
    int received_length;
    if((received_length = recv(socket_desc, server_reply, 2000, 0)) < 0)
    {
        printf("\n Error : Could not receive from server \n");
    }
    else printf("Reply received\n");
    printf("%d\n", received_length);
    
    int write = 0;
    FILE *fp;
    if((fp = fopen(argv[6], "w")) == NULL){
        printf("\n Error : Could not open file to write the html \n");
    }
    else{
        for(int i=0; i<strlen(server_reply); i++) {
            if(server_reply[i] == '<')   write = 1;
            if(write == 1) {
                fputc(server_reply[i], fp);
            }
        } 
        fputs("\n", fp); 
        fclose(fp);
    }
    // html written to file
    printf("html written to the file\n");
    close(socket_desc);
    

/*
GET /cc.gif HTTP/1.1
Host: info.in2p3.fr


*/
    
    //<IMG SRC="cc.gif">
    if(image) {
        char image_src[200];
        char *first_pos;
        first_pos = strstr(server_reply, "SRC");
        int start_of_source = first_pos - server_reply;
        start_of_source +=5;
        for(int i=start_of_source; server_reply[i] != '\"'; i++){
            image_src[i-start_of_source] = server_reply[i];
        }
        printf("\n image source is /%s\n", image_src);

        int socket_desc_img;
        if((socket_desc_img = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Error : Could not create socket \n");
            return 1;
        }
        if (connect(socket_desc_img , (struct sockaddr *)&serv_addr , sizeof(serv_addr)) < 0)
        {
            printf("\n Error : Could not connect to server \n");
            return 1;
        }
        printf("Connected\n");

        char image_request[2000];
        char image_reply[500000];
        memset(image_request, '0', sizeof(image_request));
        sprintf(image_request, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", image_src, host_name);
        if(send(socket_desc_img, image_request, strlen(image_request), 0) < 0)
        {
            printf("\n Error : Could not send image_request to server \n");
            return 1;
        }
        else printf("Data Send\n");
        
        //Receive a reply from the server
        if((received_length = recv(socket_desc_img, image_reply, 2000, 0)) < 0)
        {
            printf("\n Error : Could not receive image from server \n");
        }
        else printf("Reply received\n");
        printf("%d\n", received_length);
        
        int write = 0;
        if((fp = fopen(argv[7], "w")) == NULL){
            printf("\n Error : Could not open file to save the image \n");
        }
        else{
            write = 0;
            for(int i=0; i<received_length; i++) {
                // if(server_reply[i] == '<')   write = 1;
                // if(write == 1) {
                //    fputc(server_reply[i], fp);
                // }
            } 
            fputs(server_reply, fp);
            // fputs("\n", fp); 
            fclose(fp);
        }
        // html written to file
        printf("image saved to the file\n");
        close(socket_desc_img);
    }
    

    




    //redirect
    //1. Location: /redirect1
    //2. Found. Redirecting to /redirect1
    
    //Connection closed by foreign host.



    return 0;
}
