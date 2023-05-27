#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
pthread_cond_t wait;
void Eroare(char *e)
{
    perror(e);
    exit(1);
}

int main()
{
    int sock;
    struct sockaddr_in server;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
    server.sin_port = htons(2500);
    int status = connect(sock, (struct sockaddr *)&server, sizeof(server));
    if (status == 0)
        printf("Sunteti conectat! \n");
    else
        Eroare("Eroare de conexiune! Conexiunea nu s-a putut realiza.");
    char *sendBuff = NULL;
    char receiveBuff[1025];
    printf("Comanda ");
    char buffer_to_send[1025];
    fgets(buffer_to_send, 1025, stdin);
    buffer_to_send[strlen(buffer_to_send) - 1] = '\0';
    char *p = strtok(buffer_to_send, " ");
    int size = 0;
    sendBuff = (char *)malloc(strlen(p) + 1);
    memcpy(sendBuff, p, strlen(p) + 1);
    size = strlen(p) + 1;
    if (strcmp(p, "LIST") == 0)
        send(sock, "LIST\0", 5, 0);
    if (strcmp(p, "LIST") != 0)
    {

        if (strcmp(p, "GET") == 0 || strcmp(p, "DELETE") == 0)
        {
            p = strtok(NULL, " ");
            if (p != NULL)
            {
                int len = (int)strlen(p);
                char num[10];
                sprintf(num, "%d", len);
                char *aux = (char *)realloc(sendBuff, size + strlen(num) + strlen(p) + 2);
                memcpy(aux, sendBuff, size);
                memcpy(aux + size, num, strlen(num) + 1);
                size += strlen(num) + 1;

                memcpy(aux + size, p, strlen(p) + 1);
                sendBuff = aux;
                size += (strlen(p) + 1);
            }
            else
            {
                printf("Error\n");
                exit(0);
            }
            send(sock, sendBuff, size, 0);
        }
        else if (strcmp(p, "PUT") == 0)
        {
            p = strtok(NULL, " ");
            if (p != NULL)
            {
                send(sock, "PUT\0", 4, 0);   // operatie
                send(sock, p, strlen(p), 0); // lungime nume fisier
                send(sock, "\0", 1, 0);      // delimiter
                p = strtok(NULL, " ");
                send(sock, p, strlen(p), 0); // nume fisier
                send(sock, "\0", 1, 0);      // delimiter
                p = strtok(NULL, " ");
                send(sock, p, strlen(p), 0); // lungime fisier
                send(sock, "\0", 1, 0);
                int size = atoi(p);
                char *content = (char *)calloc(size, sizeof(char));
                p = strtok(NULL, " ");
                int offset = 0;
                while (p != NULL)
                {

                    strcat(content, p);
                    if (p != NULL)
                        strcat(content, " ");
                    p = strtok(NULL, " ");
                }
                send(sock, content, size, 0); // continut
            }
            else
            {
                printf("Error\n");
                exit(0);
            }
        }
        else if (strcmp(p, "UPDATE") == 0)
        {
            p = strtok(NULL, " ");
            // send(sock,"UPDATE\0004\000fis4\0003\0003\000ana",22,0);
            if (p != NULL)
            {
                send(sock, "UPDATE\0", 7, 0); // operatie
                send(sock, p, strlen(p), 0);  // lungime nume fisier
                send(sock, "\0", 1, 0);       // delimiter
                p = strtok(NULL, " ");
                send(sock, p, strlen(p), 0); // nume fisier
                send(sock, "\0", 1, 0);      // delimiter
                p = strtok(NULL, " ");
                send(sock, p, strlen(p), 0); // startbyte
                send(sock, "\0", 1, 0);
                p = strtok(NULL, " ");
                send(sock, p, strlen(p), 0); // dimensiune
                send(sock, "\0", 1, 0);
                int size = atoi(p);
                char *content = (char *)calloc(size, sizeof(char));
                p = strtok(NULL, " ");
                int offset = 0;
                while (p != NULL)
                {

                    strcat(content, p);
                    if (p != NULL)
                        strcat(content, " ");
                    p = strtok(NULL, " ");
                }
                send(sock, content, size, 0); // continut
                // send(sock,"UPDATE\0004\000fis5\0001\0003\000ana",22,0);
            }
            else
            {
                printf("Error\n");
                exit(0);
            }
        }
        else if (strcmp(p, "SEARCH") == 0)
        {
            send(sock, "SEARCH\0", 7, 0);
            p = strtok(NULL, " \n");
            char len[10];
            sprintf(len, "%ld", strlen(p));
            send(sock, len, strlen(len) + 1, 0);
            send(sock, p, strlen(p), 0);
        }
    }
    recv(sock, &receiveBuff, sizeof(receiveBuff), 0);
    printf("\n");
    exit(0);
    char stat[2];
    strcpy(stat, receiveBuff);
    printf("Status:%s\n", stat);
    if (strcmp(stat, "5") == 0 || strcmp(stat, "6") == 0)
    {
        exit(0);
    }
    if (strcmp(stat, "7") == 0)
    {
        int offset = 2;
        while (receiveBuff + offset != NULL)
        {
            printf("%s\n", receiveBuff + offset);
            offset += strlen(receiveBuff + offset) + 1;
        }
        exit(0);
    }
    if (strcmp(stat, "2") != 0 && strcmp(stat, "3") != 0 && strcmp(stat, "4") != 0)
    {
        char lungime[10];
        strcpy(lungime, receiveBuff + strlen(stat) + 1);
        printf("Bytes:%s\n", lungime);
        int offset = strlen(stat) + strlen(lungime) + 2;
        int bytes;
        sscanf(lungime, "%d", &bytes);
        char filename[50];
        while (bytes > 0)
        {
            strcpy(filename, receiveBuff + offset);
            offset += strlen(filename) + 1;
            bytes -= (strlen(filename) + 1);
            printf("%s\n", filename);
        }
    }

    memset(receiveBuff, 0, sizeof(receiveBuff));

    shutdown(sock, SHUT_RDWR);
    close(sock);
}