#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/sendfile.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include "structuri.h"
__thread int clientsock;
__thread char fisier[50];
__thread char cuvant[20];
pthread_mutex_t update = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t change = PTHREAD_COND_INITIALIZER;
int changed = 0;
int nr_files = 0;
struct lista *liste = NULL;
void Eroare(char *e)
{
    perror(e);
    exit(1);
}
void getnrfiles()
{
    nr_files = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir("./files");

    if (d)
    {
        dir = readdir(d);
        while (dir != NULL)
        {

            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            {
                dir = readdir(d);
                continue;
            }
            if (dir->d_type == DT_REG)
            {
                nr_files++;
            }
            dir = readdir(d);
        }
    }
    closedir(d);
}
void sort(char ***cuv, int **ap, int nr_files)
{
    for (int i = 0; i < nr_files - 1; i++)
        for (int j = i + 1; j < nr_files; j++)
        {
            if ((*ap)[i] < (*ap)[j])
            {
                char aux[50];
                strcpy(aux, *cuv[i]);
                strcpy(*cuv[i], *cuv[j]);
                strcpy(*cuv[j], aux);
                int aux1 = (*ap)[i];
                (*ap)[i] = (*ap)[j];
                (*ap)[j] = aux1;
            }
        }
}
void *WORDS(void *arg)
{
    pthread_mutex_lock(&update);
    if (liste != NULL)
        free(liste);
    getnrfiles();
    DIR *d;
    struct dirent *dir;
    d = opendir("./files");
    liste = (struct lista *)malloc(nr_files * sizeof(struct lista));
    int file = 0;
    if (d)
    {
        dir = readdir(d);
        while (dir != NULL)
        {
            if (strcmp(dir->d_name, ".") == 0 | strcmp(dir->d_name, "..") == 0)
            {
                dir = readdir(d);
                continue;
            }
            if (dir->d_type == DT_REG)
            {
                char filename[50];
                strcpy(filename, "./files/");
                strcat(filename, dir->d_name);
                FILE *f = fopen(filename, "r");
                char buff[1500];
                char **cuv = NULL;
                int *aparitii = NULL;
                int nr_cuv = 0;
                while (fgets(buff, 1500, f))
                {
                    char *p = strtok(buff, " \n");
                    while (p != NULL)
                    {
                        if (cuv == NULL)
                        {
                            cuv = (char **)malloc((nr_cuv + 1) * sizeof(char *));
                            cuv[0] = (char *)malloc(strlen(p));
                            aparitii = (int *)malloc((nr_cuv + 1) * sizeof(int));
                            strcpy(cuv[0], p);
                            aparitii[0] = 1;
                            nr_cuv++;
                        }
                        else
                        {
                            int ok = 0;
                            for (int i = 0; i < nr_cuv; i++)
                            {
                                if (strcmp(cuv[i], p) == 0)
                                {
                                    ok = 1;
                                    aparitii[i] = aparitii[i] + 1;
                                    break;
                                }
                            }
                            if (ok == 0)
                            {
                                char **aux = (char **)realloc(cuv, (nr_cuv + 1) * sizeof(char *));
                                int *aux1 = (int *)realloc(aparitii, (nr_cuv + 1) * sizeof(int));
                                cuv = aux;
                                aparitii = aux1;
                                cuv[nr_cuv] = (char *)malloc(strlen(p));
                                strcpy(cuv[nr_cuv], p);
                                aparitii[nr_cuv] = 1;
                                nr_cuv++;
                            }
                        }
                        p = strtok(NULL, " \n");
                    }
                }
                sort(&cuv, &aparitii, nr_cuv);
                liste[file].nr_cuv = (10 < nr_cuv) ? 10 : nr_cuv;
                for (int j = 0; j < liste[file].nr_cuv; j++)
                {
                    strcpy(liste[file].cuvinte[j], cuv[j]);
                }
                strcpy(liste[file].filename, dir->d_name);
            }
            dir = readdir(d);
            file++;
        }
    }
    changed = 1;
    pthread_cond_signal(&change);
    pthread_mutex_unlock(&update);
}
void *LOG(void *arg)
{
    struct to_log *param = (struct to_log *)arg;
    FILE *f = fopen("logfile.txt", "a");
    printf("In the log\n");
    time_t timp = time(NULL);
    struct tm ts;
    ts = *localtime(&timp);
    char buf[150];
    strftime(buf, sizeof(buf), "%Y-%m-%d, %H:%M:%S", &ts);
    if (strcmp(param->operatie, "") != 0)
    {
        strcat(buf, ", ");
        strcat(buf, param->operatie);
    }
    if (strcmp(param->fisier, "") != 0)
    {
        strcat(buf," ");
        strcat(buf, param->fisier);
    }
    if (strcmp(param->cuvant, "") != 0)
    {
        strcat(buf," ");
        strcat(buf, param->cuvant);
    }
    fprintf(f, "%s\n", buf);
    fclose(f);
}
void LIST()
{
    pthread_t log;
    struct to_log param;
    strcpy(param.operatie, "LIST");
    strcpy(param.fisier, "");
    strcpy(param.cuvant, "");
    pthread_create(&log, NULL, LOG, &param);
    pthread_join(log, NULL);
    DIR *d;
    struct dirent *dir;
    d = opendir("./files");
    if (d)
    {
        dir = readdir(d);
        if (dir == NULL)
        {
            char mess[4] = "1\0";
            memcpy(mess + 2, "0\0", 2);
            send(clientsock, mess, 4, 0);
            return;
        }
        int nrbytes = 0;
        char *tosend = (char *)malloc(sizeof(dir->d_name) + 1);
        while (dir != NULL)
        {
            if (strcmp(dir->d_name, ".") == 0 | strcmp(dir->d_name, "..") == 0)
            {
                dir = readdir(d);
                continue;
            }
            if (dir->d_type == DT_DIR)
            {
                char *aux = (char *)malloc(nrbytes + strlen(dir->d_name) + 3);
                memcpy(aux, tosend, nrbytes);
                strcpy(aux + nrbytes, "./");
                strcpy(aux + nrbytes + 2, dir->d_name);
                free(tosend);
                tosend = NULL;
                tosend = aux;
                nrbytes += (3 + strlen(dir->d_name));
            }
            if (dir->d_type == DT_REG)
            {
                char *aux = (char *)malloc(nrbytes + strlen(dir->d_name) + 1);
                memcpy(aux, tosend, nrbytes);
                strcpy(aux + nrbytes, dir->d_name);
                free(tosend);
                tosend = NULL;
                tosend = aux;
                nrbytes += (1 + strlen(dir->d_name));
            }
            dir = readdir(d);
        }
        send(clientsock, "0\0", 2, 0);
        char size[11];
        sprintf(size, "%d", nrbytes);
        send(clientsock, size, strlen(size) + 1, 0);
        send(clientsock, tosend, nrbytes, 0);
        printf("JUST sended\n");
        closedir(d);
    }
}
void GET()
{
    pthread_t log;
    struct to_log param;
    strcpy(param.operatie, "GET");
    strcpy(param.fisier, fisier);
    strcpy(param.cuvant, "");
    pthread_create(&log, NULL, LOG, &param);
    pthread_join(log, NULL);
    FILE *f = fopen(fisier, "r");
    if (f == NULL)
    {
        printf("EROARE\n");
        char mess[20] = "1\0";
        memcpy(mess + 2, "14\0EROARE-FISIER\0", 17);
        send(clientsock, mess, 19, 0);
        return;
    }
    int size = 0;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fclose(f);
    if (size != 0)
    {
        char num[11];
        sprintf(num, "%d", size);
        char *buffer_to_send = malloc(strlen(num) + 3);
        memcpy(buffer_to_send, "0\0", 2);
        memcpy(buffer_to_send + 2, num, strlen(num) + 1);
        send(clientsock, buffer_to_send, strlen(num) + 3, 0);
        int fd = open(fisier, O_RDONLY);
        int send = 0;
        while (send < size)
        {
            send += sendfile(clientsock, fd, NULL, size);
        }
        close(fd);
    }
    else
    {
        char mess[4] = "0\0";
        memcpy(mess + 2, "0\0", 2);
        send(clientsock, mess, 4, 0);
    }
}

void DELETE()
{
    pthread_t log;
    struct to_log param;
    strcpy(param.operatie, "DELETE");
    strcpy(param.fisier, fisier);
    strcpy(param.cuvant, "");
    pthread_create(&log, NULL, LOG, &param);
    pthread_join(log, NULL);
    FILE *f = fopen(fisier, "r");
    if (f == NULL)
    {
        char mess[20] = "4\0";
        send(clientsock, mess, 2, 0);
        return;
    }
    fclose(f);
    int success = rmdir(fisier);
    if (success == 0)
        send(clientsock, "2\0", 2, 0);
    else
        send(clientsock, "3\0", 2, 0);
}
void PUT(int size, char *content)
{
    pthread_t log;
    struct to_log param;
    strcpy(param.operatie, "PUT");
    strcpy(param.fisier, fisier);
    strcpy(param.cuvant, "");
    pthread_create(&log, NULL, LOG, &param);
    pthread_join(log, NULL);
    int fd = open(fisier, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int sent = 0;
    while (sent < size)
    {
        sent += write(fd, content + sent, size - sent);
    }
    close(fd);
    send(clientsock, "5", 1, 0);
}

void UPDATE(int startbyte, int dimensiune, char *content)
{
    pthread_t log;
    struct to_log param;
    strcpy(param.operatie, "UPDATE");
    strcpy(param.fisier, fisier);
    strcpy(param.cuvant, "");
    pthread_create(&log, NULL, LOG, &param);
    pthread_join(log, NULL);
    int fd = open(fisier, O_WRONLY);
    lseek(fd, (off_t)startbyte, SEEK_SET);
    int sent = 0;
    while (sent < dimensiune)
    {
        sent += write(fd, content + sent, dimensiune - sent);
    }
    close(fd);
    send(clientsock, "6", 1, 0);
}
void SEARCH(char *cuvant)
{
    pthread_t log;
    struct to_log param;
    strcpy(param.operatie, "SEARCH");
    strcpy(param.fisier, "");
    strcpy(param.cuvant, cuvant);
    pthread_create(&log, NULL, LOG, &param);
    pthread_join(log, NULL);
    send(clientsock, "7\0", 2, 0);
    for (int i = 0; i < nr_files; i++)
    {
        int ok = 0;
        for (int j = 0; j < liste[i].nr_cuv; j++)
            if (strcmp(liste[i].cuvinte[j], cuvant) == 0)
            {
                send(clientsock, liste[i].filename, strlen(liste[i].filename), 0);
                send(clientsock,"\0",1,0);
                puts(liste[i].filename);
                ok = 1;
                break;
            }
        if (ok == 0)
        {
            char filename[50];
            strcpy(filename, "./files/");
            strcat(filename, liste[i].filename);
            FILE *f = fopen(filename, "r");
            char buf[1500];
            while (fgets(buf, 1500, f))
            {
                char *p = strtok(buf, " \n");
                while (p != NULL)
                {
                    if (strcmp(p, cuvant) == 0)
                    {
                        ok = 1;
                        send(clientsock, liste[i].filename, strlen(liste[i].filename) + 1, 0);
                        puts(liste[i].filename);
                        break;
                    }
                    p = strtok(NULL, " \n");
                }
            }
        }
    }
}
void *AdministrateClient(void *params)
{
    parameters_t *par = (parameters_t *)params;
    char message_from_client[1025];
    clientsock = par->clientsock;
    int status = -1;
    do
    {
        status = recv(clientsock, &message_from_client, 1025, 0);
        if (strcmp(message_from_client, "LIST") == 0)
        {
            LIST();
        }
        else if (strstr(message_from_client, "GET") != NULL)
        {
            char len[10];
            sscanf(message_from_client + 4, "%s", len);
            int nr = atoi(len);
            if (nr != 0)
            {
                char p[50];
                sscanf(message_from_client + 5 + strlen(len), "%s", p);

                char complete[50];
                strcpy(complete, "./files/");
                strcat(complete, p);
                strcpy(fisier, complete);
            }
            GET();
        }
        else if (strstr(message_from_client, "DELETE") != NULL)
        {
            char len[10];
            sscanf(message_from_client + 7, "%s", len);
            int nr = atoi(len);
            char p[50];
            sscanf(message_from_client + 8 + strlen(len), "%s", p);
            char complete[50];
            strcpy(complete, "./files/");
            strcat(complete, p);
            strcpy(fisier, complete);
            DELETE();
        }
        else if (strstr(message_from_client, "PUT") != NULL)
        {
            char len[10];
            sscanf(message_from_client + 4, "%s", len);
            int offset = strlen(len) + 5;
            strcpy(fisier, "./files/");
            strcat(fisier, message_from_client + offset);
            offset += strlen(message_from_client + offset) + 1;
            strcpy(len, message_from_client + offset);
            int size = atoi(len);
            offset += strlen(len) + 1;
            char *content = (char *)malloc(size * sizeof(char)); // PUT\04\0fis4\03\0ana
            memcpy(content, message_from_client + offset, size);
            if (strcmp(content, "\0") == 0)
            {
                status = recv(clientsock, content, size, 0);
            }
            PUT(size, content);
        }
        else if (strstr(message_from_client, "UPDATE") != NULL)
        {
            char len[10];
            sscanf(message_from_client + 7, "%s", len);
            int offset = strlen(len) + 8;
            strcpy(fisier, "./files/");
            strcat(fisier, message_from_client + offset);
            offset += strlen(message_from_client + offset) + 1;
            char startbyte[10];
            sscanf(message_from_client + offset, "%s", startbyte);
            offset += strlen(startbyte) + 1;
            sscanf(message_from_client + offset, "%s", len);
            offset += strlen(len) + 1;
            char *content = (char *)malloc(atoi(len) * sizeof(char));
            memcpy(content, message_from_client + offset, atoi(len));

            UPDATE(atoi(startbyte), atoi(len), content);
        }
        else if (strstr(message_from_client, "SEARCH") != NULL)
        {
            char len[10];
            sscanf(message_from_client + 7, "%s", len);
            char *cuvant = (char *)malloc(atoi(len) * sizeof(char));
            strcpy(cuvant, message_from_client + strlen(len) + 8);
            SEARCH(cuvant);
        }
        else
        {
            /*message_to_client=(char*)malloc(15);
            strcpy(message_to_client, "Necunoscut");*/
        }
        memset(message_from_client, 0, sizeof(message_from_client));
    } while (status != 0);
    close(clientsock);
}

int main()
{

    pthread_mutex_lock(&update);
    pthread_t to_update;
    pthread_create(&to_update, NULL, WORDS, NULL);
    while (changed == 0)
        pthread_cond_wait(&change, &update);
    printf("Listen...\n");
    struct sockaddr_in server;
    int readSock, connSock;
    readSock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(2500);

    bind(readSock, (struct sockaddr *)&server, sizeof(server));
    listen(readSock, 10);
    int i = 0;
    while (1)
    {
        struct sockaddr_in client;
        int length = sizeof(client);
        connSock = accept(readSock, (struct sockaddr *)&client, &length);
        pthread_t administration;
        parameters_t param;
        param.server = &server;
        param.client = &client;
        param.readsock = readSock;
        param.clientsock = connSock;
        pthread_create(&administration, NULL, AdministrateClient, &param);
    }
    close(readSock);
    pthread_mutex_unlock(&update);
}