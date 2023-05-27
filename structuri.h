struct parameters
{
    struct sockaddr_in *server;
    struct sockaddr_in *client;
    int readsock;
    int clientsock;
};
struct lista
{
    char filename[50];
    char cuvinte[10][50];
    int nr_cuv;
};
struct to_log
{
    char operatie[20];
    char fisier[50];
    char cuvant[20];
};
typedef struct parameters parameters_t;