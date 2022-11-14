/*tcp:server端*/
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
#include <signal.h>
#include <limits.h>
#include "hash.h"
#define BUFFER_SIZE 1024

HashTable* ht;
char *commands;

void put(char *key,char *value)
{
    ht_insert(ht,key,value);
}

void get(char *key)
{
    print_search(ht,key);
}

void delete(char *key)
{
    ht_delete(ht, key);
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int status = 0;//How many clients

struct ps
{
    int st;
    pthread_t *thr;
};

void catch_Signal(int Sign)
{
    switch(Sign)
    {
    case SIGINT:
        printf("SYS  MSG:SERVER SHUTDOWN \n");
        exit(EXIT_SUCCESS);
    }
}

int signal1(int signo,void (*func)(int))
{
    struct sigaction act,oact;
    act.sa_handler = func;//回調函數初始化
    sigemptyset(&act.sa_mask);//初始化
    act.sa_flags = 0;
    return sigaction(signo,&act,&oact);
}

void *recvsocket(void *arg)//接收client端socket數據的線程
{
    struct ps *p = (struct ps *)arg;
    int st = p->st;
    printf("[THREAD INFO] Thread %lu created, serving connection fd->%d.\n",pthread_self(),st);
    char s[1024];
    while(1)
    {
        memset(s, 0, sizeof(s));
        int rc = recv(st, s, sizeof(s), 0);
        if (rc <= 0)//如果recv返回小於等於0，代表socket已經關閉或者出錯了
        {
            if(rc == 0)
            {
                printf("[CLIENT QUIT] ");
                if(status >= 0)
                {
                    pthread_mutex_lock(&mutex);
                    status-- ;
                    printf("Online User: %d\n",status);
                    pthread_mutex_unlock(&mutex);
                }
            }
            break;
        }

        struct sockaddr_in client_addr;
        memset(&client_addr,0,sizeof(client_addr));
        socklen_t len = sizeof(client_addr);
        getpeername(st, (struct sockaddr *) &client_addr, &len);

        //打印client傳送來的內容
        char *token=strtok(s," ");
        char *text[100];
        int count=0;
        text[0]="";
        text[1]="";
        text[2]="";
        //Deal with text receive from client
        while(token!=NULL)
        {
            text[count++]=token;
            token=strtok(NULL," ");
        }
        if(count==1)
        {
            text[0]=strtok(text[0],"\n");
        }
        else if(count==2)
        {
            text[1]=strtok(text[1],"\n");
        }
        else
        {
            text[2]=strtok(text[2],"\n");
        }
        //What to do depend on instruction
        if(strcmp(text[0],"HELP")==0)
        {
            send(st, commands, strlen(commands), 0);

        }
        else if(strcmp(text[0],"EXIT")==0)
        {
            if( count == 1)
            {
                printf("[CLIENT QUIT] ");
                if(status >= 0)
                {
                    pthread_mutex_lock(&mutex);
                    status-- ;
                    printf("Online User: %d\n",status);
                    pthread_mutex_unlock(&mutex);
                }
                close(st);
            }
            else
            {
                send(st, "Unavailable instruction!\n", strlen("Unavailable instruction!\n"), 0);
            }
        }
        else if(strcmp(text[0],"DELETE")==0)
        {
            if( count == 2)
            {
                delete(text[1]);
                send(st, sent_string, strlen(sent_string), 0);
            }
            else
            {
                send(st, "Unavailable instruction!\n", strlen("Unavailable instruction!\n"), 0);
            }

        }
        else if(strcmp(text[0],"PUT")==0)
        {
            if( count == 3)
            {
                put(text[1],text[2]);
                send(st, sent_string, strlen(sent_string), 0);
            }
            else
            {
                send(st, "Unavailable instruction!\n", strlen("Unavailable instruction!\n"), 0);
            }

        }
        else if(strcmp(text[0],"GET")==0 )
        {
            if( count == 2)
            {
                get(text[1]);
                send(st, sent_string, strlen(sent_string), 0);
            }
            else
            {
                send(st, "Unavailable instruction!\n", strlen("Unavailable instruction!\n"), 0);
            }

        }
        else
        {
            send(st, "Unavailable instruction!\n", strlen("Unavailable instruction!\n"), 0);
        }
    }
    pthread_cancel(*(p->thr));//被cancel掉的線程內部沒有使用鎖。
    return NULL;
}

void *sendsocket(void *arg)//向client端socket發送數據的線程
{
    int st = *(int *)arg;
    char *s="INPUT";
    send(st, s, strlen(s), 0);
    /*char s[1024];

    while(1)
    {
        memset(s, 0, sizeof(s));
        read(STDIN_FILENO, s, sizeof(s));//從鍵盤讀取用戶輸入信息
        send(st, s, strlen(s), 0);
    }*/
    return NULL;
}


void authorInfo()
{
    puts("=====================================================");
    puts("Name        : Simple Key-value store in Linux");
    puts("Version     : HW2 v1.0 ");
    puts("Copyright   : Power by JudyFang");
    puts("Description : Multithread && Socket programming in C");
    puts("=====================================================");
}


int main(int argc, char **argv)
{
    ht=create_table(CAPACITY);
    //input="Init";
    authorInfo();
    char *server_port = 0;
    int opt = 0;
    /* Parsing args */
    while ((opt = getopt(argc, argv, "p:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            server_port = malloc(strlen(optarg) + 1);
            strncpy(server_port, optarg, strlen(optarg));
            break;
            /*case '?':
                fprintf(stderr, "Unknown option \"-%c\"\n", isprint(optopt) ?optopt : '#');*/
            return 0;
        }
    }

    if (!server_port)
    {
        fprintf(stderr, "Error! No port number provided!\n");
        exit(1);
    }

    int port = atoi(server_port);
    int st = socket(AF_INET, SOCK_STREAM, 0);//初始化socket

    //給TCP設置地址可重用
    int on = 1;
    if (setsockopt(st, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
    {
        printf("setsockopt failed %s\n", strerror(errno));
        return EXIT_FAILURE;
    }


    struct sockaddr_in addr;//定義一個IP地址結構
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;//將addr結構的屬性定位為TCP/IP地址
    addr.sin_port = htons(port);//將本地字節順序轉化為網絡字節順序。
    addr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY代表這個server上所有的地址

    //將IP與server程序綁定
    if (bind(st, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        printf("bind failed %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    //server端開始listen，
    if (listen(st, 20) == -1)
    {
        printf("listen failed %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    int client_st = 0;//client端socket
    //socklen_t len = 0;
    struct sockaddr_in client_addr;//表示client端的IP地址
    //void *p = &client_addr;

    //Commands Table
    char *a="Commands            Description\n";
    char *b="PUT [key] [value]   Store the key value pair ([key], [value]) into the database.\n";
    char *c="GET [key]           Get the value of [key] from the database.\n";
    char *d="DELETE [key]        Delete [key] and it's associated value from the database.\n";
    char *e="EXIT		    Exit.\n";
    commands=concat(a,b);
    commands=concat(commands,c);
    commands=concat(commands,d);
    commands=concat(commands,e);

    //Create Thread
    pthread_t thrd1, thrd2;
    signal1(SIGINT,catch_Signal);
    printf("[INFO] Start with a clean database...\n");
    printf("[INFO] Initializing the server...\n");
    printf("[INFO] Server initialized!\n");
    printf("[INFO] Listening on the port %s...\n",server_port);

    while (1)
    {
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t len = sizeof(client_addr);
        //accept會阻塞，直到有客戶端連接過來，accept返回client的socket描述符
        client_st = accept(st, (struct sockaddr *)&client_addr, &len);
        pthread_mutex_lock(&mutex);//為全局變量加一個互斥鎖，防止與線程函數同時讀寫變量的衝突
        status++;
        printf("[CLIENT CONNECTED] User Login,Online User: %d\n",status);
        pthread_mutex_unlock(&mutex);//解鎖
        if (status > 100)//代表這是下一個socket連接
        {
            char tip[] = "The Number Of Users Is Limited.Please Wait.\n";
            send(client_st, tip, strlen(tip), 0);
            close(client_st);
            pthread_mutex_lock(&mutex);//為全局變量加一個互斥鎖，防止與線程函數同時讀寫變量的衝突
            status--;
            printf("[CLIENT CONNECTED] Online User: %d\n",status);
            pthread_mutex_unlock(&mutex);//解鎖
            continue;
        }

        if (client_st == -1)
        {
            printf("accept failed %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        printf("[INFO] Listening on the port %s...\n",server_port );
        //printf("[CLIENT CONNECTED]:Accept by client_IP:%s\n\n",inet_ntoa(client_addr.sin_addr));
        struct ps ps1;
        ps1.st = client_st;
        ps1.thr = &thrd2;
        //設置線程為可分離
        pthread_create(&thrd1, NULL, recvsocket, &ps1);
        pthread_detach(thrd1);

        //設置線程為可分離
        /*pthread_create(&thrd2, NULL, sendsocket, &ps1);
        pthread_detach(thrd2);*/
    }
    close(st);//關閉server端listen的socket

    return EXIT_SUCCESS;
}


