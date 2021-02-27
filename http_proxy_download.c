/* f20180195@hyderabad.bits-pilani.ac.in Arpan Sarangi */

/* Brief description of program...*/
/* ... */

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
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

    // base64encode
    char encoding[64];
    int j=0;
    for(int j=0; j<26; j++){
        encoding[j] = j + 'A';
    }
    for(int j=0; j<26; j++){
        encoding[j+26] = j + 'a';
    }
    for(int j=0; j<10; j++){
        encoding[j+52] = j + '0';
    }
    encoding[62] = '+', encoding[63] = '/';
    // printf("%s\n", encoding);
    char input[60];
    j=0;
    for(int i=0; argv[4][i]!='\0'; i++){
        input[j++] = argv[4][i];
    }
    input[j++] = ':';
    for(int i=0; argv[5][i]!='\0'; i++){
        input[j++] = argv[4][i];
    }

    int input_powers[] = {128, 64, 32, 16, 8, 4, 2, 1};
    int output_powers[] = {32, 16, 8, 4, 2, 1};
    char binary_form[500];
    int b=0;
    for(int k=0; k<j; k++){
        int pos=(int)input[k];
        if(pos == -1) {
            printf("\n Error: Username or password has characters which are not allowed \n");
            return 1;
        }
        for(int i=0; i<8; i++){
            if(input_powers[i] <= pos){
                binary_form[b++] = '1';
                pos -= input_powers[i];
            }
            else    binary_form[b++] = '0';
        }
    }
    char output[80];
    int o=0;
    int new_base = 0; 
    for(int k=0; k<b; k++){
        int l = k%6;
        if(l == 0 && k>0){
            output[o++] = encoding[new_base];
            new_base = 0;
        }  
        if(binary_form[k] == '1')   new_base += output_powers[l];
    }
    output[o++] = encoding[new_base];
    if(o%4 == 2){
        output[o++] = '=';
        output[o++] = '=';
    }
    else if(o%4 == 3){
        output[o++] = '=';
    }
    printf("%s\n", output);


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
    // struct hostent *he;
    char host_name[100];
    memset(host_name, '0', sizeof(host_name));
    strcpy(host_name, argv[1]);
    // if((he = gethostbyname(host_name)) == NULL) {
    //     printf("\n Error : Could not get ip of host \n");
    //     return 1;
    // }
    char ip[30], port[10];
    strcpy(ip, argv[2]);
    strcpy(port, argv[3]);
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    // memcpy(&serv_addr.sin_addr, he -> h_addr_list[0], he -> h_length);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));


    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&serv_addr , sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Could not connect to server \n");
        return 1;
    }
    printf("Connected\n");
    
// GET http://info.in2p3.fr/ HTTP/1.1
// Proxy-Authorization: Basic Y3NmMzAzOmNzZjMwMw== 

    char server_reply[5000];
    int received_length;
    int write = 0, total = 0, content_length = 10000;
    
    FILE *fp;
    int first;
    char request[2000];
    memset(request, '0', sizeof(request));
    sprintf(request, "GET http://%s/ HTTP/1.1\r\nProxy-Authorization: Basic %s\r\n\r\n", argv[1], output);
    g:
    if(send(socket_desc, request, strlen(request), 0) < 0)
    {
        printf("\n Error : Could not send request to server \n");
        return 1;
    }
    else printf("Data Send\n");
    
    //Receive a reply from the server
    memset(server_reply, '0', sizeof(server_reply));
    if((fp = fopen(argv[6], "w")) == NULL){
        printf("\n Error : Could not open file to write the html \n");
    }
    first = 1;
    do
    {
        received_length = recv(socket_desc, server_reply, 5000, 0);
        if(first == 1){
            if(server_reply[9] == '3'){
                char redirect[50];
                int j = 0;
                char *loc;
                loc = strstr(server_reply, "Location: ");
                int location = 10 + (loc - server_reply);
                for(int i=location; server_reply[i] != '\r' && server_reply[i] != '\n' && i < received_length; i++){
                    redirect[j++] = server_reply[i];
                }
                redirect[j] = '\0';
                fclose(fp);
                sprintf(request, "GET %s HTTP/1.1\r\nProxy-Authorization: Basic %s\r\n\r\n", redirect, output);
                printf("\nRedirected to %s\n", redirect);
                goto g;
            }
            char *bookmark;
            bookmark = strstr(server_reply, "Content-Length: ");
            int mark = bookmark - server_reply;
            mark += 16;
            int sum = 0;
            for(int i=mark; server_reply[i] != '\r' && server_reply[i] != '\n' && i < received_length; i++){
                sum = sum * 10 + (server_reply[i] - '0');
            }
            content_length = sum;
            printf("\ncontent length is %d\n", content_length);
        }
        
        printf("%d\n", received_length);
        for(int i=0; i<received_length; i++) {
            if(server_reply[i] == '<')   write = 1;
            if(write == 1) {
                total++;
                fputc(server_reply[i], fp);
            }
        } 
        first = 0;
        if(total >= content_length)   break;
        memset(server_reply, '0', sizeof(server_reply));
    } while (received_length);
    // fputs("\n", fp); 
    fclose(fp);
    
    
    // html written to file
    printf("html written to the file\n");
    close(socket_desc);
    
    
    //<IMG SRC="cc.gif">
    if(image == 1) {
        if(argc == 7) {
            printf("\n Error : File to save logo not specified \n");
            return 1;
        }
        char image_src[200];
        char *first_pos;
        first_pos = strstr(server_reply, "SRC");
        int start_of_source = first_pos - server_reply;
        start_of_source += 5;
        for(int i=start_of_source; server_reply[i] != '\"'; i++){
            image_src[i-start_of_source] = server_reply[i];
        }
        printf("\nImage source is /%s\n", image_src);

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
        memset(image_request, '0', sizeof(image_request));
        sprintf(image_request, "GET http://%s/%s HTTP/1.1\r\nProxy-Authorization: Basic %s\r\n\r\n", argv[1], image_src, output);
        if(send(socket_desc_img, image_request, strlen(image_request), 0) < 0)
        {
            printf("\n Error : Could not send image_request to server \n");
            return 1;
        }
        else printf("Data Send\n");
        
        //Receive a reply from the server
        char image_reply[5000];
        FILE *img_fp;
        memset(image_reply, '0', sizeof(image_reply));
        write = 0;
        if((img_fp = fopen(argv[7], "w")) == NULL){
            printf("\n Error : Could not open file to save the image \n");
            return 1;
        }
        total = 0;
        int flag = 0;
        content_length = 10000;
        first = 1;
        do
        {
            received_length = recv(socket_desc_img, image_reply, 5000, 0);
            if(first == 1){
                char *bookmark;
                bookmark = strstr(image_reply, "Content-Length: ");
                int mark = bookmark - image_reply;
                mark += 16;
                int sum = 0;
                for(int i=mark; image_reply[i] != '\r' && image_reply[i] != '\n' && i < received_length; i++){
                    sum = sum * 10 + (image_reply[i] - '0');
                }
                content_length = sum;
                printf("\ncontent length is %d\n", content_length);
            }
            printf("%d\n", received_length);
            for(int i=0; i<received_length; i++) {
                if(image_reply[i] == '\n'){
                   flag++;
                }
                if(flag == 13)   write ++;
                if(write >= 2) {
                    total++;
                   fputc(image_reply[i], img_fp);
                }
            } 
            first = 0;
            if(total >= content_length)   break;
            memset(image_reply, '0', sizeof(image_reply));
        } while(received_length);
        fclose(img_fp);

        printf("Image saved to the file, total %d bytes\n", total);
        close(socket_desc_img);
    }
    

    




    //redirect
    //1. Location: /redirect1
    //2. Found. Redirecting to /redirect1
    
    //Connection closed by foreign host.



    return 0;
}

