/*tcp:client端*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

void *recvsocket(void *arg)
{
    int st = *(int *)arg;
    char s[1024];

    while(1)
    {
        memset(s, 0, sizeof(s));
        int rc = recv(st, s, sizeof(s), 0);
        if (rc <= 0)
        {
            if(rc == 0)
            {
                printf("Server Quit!\n");
            }
            break;
        }

        printf("%s", s);
        printf("> ");
        fflush(stdout);

    }
    return NULL;
}

void *sendsocket(void *arg)
{
    int st = *(int *)arg;
    char s[1024];
    while(1)
    {
        memset(s, 0, sizeof(s));
        read(STDIN_FILENO, s, sizeof(s));
        send(st, s, strlen(s), 0);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    int opt;
    char *server_host_name = NULL, *server_port = NULL;

    /* Parsing args */
    while ((opt = getopt(argc, argv, "h:p:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            server_host_name = malloc(strlen(optarg) + 1);
            strncpy(server_host_name, optarg, strlen(optarg));
            break;
        case 'p':
            server_port = malloc(strlen(optarg) + 1);
            strncpy(server_port, optarg, strlen(optarg));
            break;
            /*case '?':
                fprintf(stderr, "Unknown option \"-%c\"\n", isprint(optopt) ?
                        optopt : '#');*/
            return 0;
        }
    }

    if (!server_host_name)
    {
        fprintf(stderr, "Error!, No host name provided!\n");
        exit(1);
    }
    if(strcmp(server_host_name,"localhost")==0)
    {
        server_host_name="127.0.0.1";
    }
    if (!server_port)
    {
        fprintf(stderr, "Error!, No port number provided!\n");
        exit(1);
    }

    int port = atoi(server_port);
    int st = socket(AF_INET, SOCK_STREAM, 0); //初始化socket，

    struct sockaddr_in addr; //定義一個IP地址的結構
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; //設置結構地址類型為TCP/IP地址
    addr.sin_port = htons(port); //指定一個端口號：1234，htons:將short類型從host字節類型到net字節類型轉化
    addr.sin_addr.s_addr = inet_addr(server_host_name); //將字符串類型的IP地址轉化為int，賦給addr結構成員.

    //調用connect連接到結構addr指定的IP地址和端口號
    if (connect(st, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        printf("connect failed %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    printf("[INFO] Connected to %s:%s.\n",server_host_name,server_port);
    printf("[INFO] Welcome! Please type HELP for available commands.\n");
    printf("> ");
    fflush(stdout);

    pthread_t thrd1, thrd2;
    pthread_create(&thrd1, NULL, recvsocket, &st);
    pthread_create(&thrd2, NULL, sendsocket, &st);
    pthread_join(thrd1, NULL);
    //pthread_join(thrd2, NULL);
    close(st); //關閉socket
    return EXIT_SUCCESS;

}

