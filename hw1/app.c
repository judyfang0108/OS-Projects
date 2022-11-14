#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
FILE *fp;
#define MAXBUFLEN 1000000
unsigned char buf[MAXBUFLEN];
int buflen=0;
void main(void)
{
    int i=0;
    buflen=0;
    fp=fopen("/proc/my_info","rb");
    if(fp!=NULL)
    {
        while(!feof(fp))
        {
            buflen=fread(buf,1,MAXBUFLEN,fp);
        }
    }
    fclose(fp);
    int a;
    int b;
    int c;
    int d;
    for(i=0; i<buflen; i++)
    {
        if(buf[i]=='V' && buf[i+1]=='e'&& buf[i+2]=='r'&& buf[i+3]=='s' &&  buf[i+4]=='i' && buf[i+5]=='o' && buf[i+6]=='n')
        {
            a=i;
            break;
        }
    }
    for(i=0; i<buflen; i++)
    {
        if(buf[i]=='C'&& buf[i+1]=='P' && buf[i+2]=='U')
        {
            b=i;
            break;
        }
    }
    for(i=0; i<buflen; i++)
    {
        if(buf[i]=='M'&& buf[i+1]=='e'&& buf[i+2]=='m'&& buf[i+3]=='o' &&  buf[i+4]=='r' && buf[i+5]=='y')
        {
            c=i;
            break;
        }
    }
    for(i=0; i<buflen; i++)
    {
        if(buf[i]=='T'&& buf[i+1]=='i'&& buf[i+2]=='m'&& buf[i+3]=='e')
        {
            d=i;
            break;
        }
    }

    printf("\nWhich information do you want?\n");
    printf("Version(v),CPU(c),Memory(m),Time(t),All(a),Exit(e)?\n");
    while(1)
    {
        char input;
        scanf("%c",&input);
        if(input=='e')
        {
            break;
        }
        else if(input=='v')
        {
            for(i=a-14; i<b-16; i++)
                printf("%c",buf[i]);
        }
        else if(input=='c')
        {
            for(i=b-16; i<c-15; i++)
                printf("%c",buf[i]);
        }
        else if(input=='m')
        {
            for(i=c-15; i<d-16; i++)
                printf("%c",buf[i]);
        }
        else if(input=='t')
        {
            for(i=d-16; i<buflen; i++)
                printf("%c",buf[i]);
        }
        else if(input=='a')
        {
            for(i=0; i<buflen; i++)
                printf("%c",buf[i]);
        }
        printf("Which information do you want?\n");
        printf("Version(v),CPU(c),Memory(m),Time(t),All(a),Exit(e)?\n");
        getchar();
    }

}


