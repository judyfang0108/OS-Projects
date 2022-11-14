#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#define MAX 1024
#define TLB_num 32

struct node
{
    char name[100];
    int frame;
    struct node*next;
} typedef Node;

struct FFL //free frame list
{
    int frame;
    int page;
    char process;
    struct FFL * next;
} typedef FFL;

struct TLBE //TLB
{
    int VPN;
    int PFN;
    int valid;
    struct TLBE * next;
} typedef TLBE;

struct PTE //Page Table
{
    int frame;
    int dbi;
    int valid;
    int reference;
    int present;
} typedef PTE;

int intArray[MAX];//記錄前一個放的值
int front = 0;
int rear = -1;
int itemCount = 0;

int peek()
{
    return intArray[front];
}

bool isEmpty()
{
    return itemCount == 0;
}

bool isFull()
{
    return itemCount == MAX;
}

int size()
{
    return itemCount;
}

void insert(int data) //從後面加item
{
    if(!isFull())
    {
        if(rear == MAX-1)
        {
            rear = -1;
        }
        intArray[++rear] = data;
        itemCount++;
    }
}

int removeData() //移除頭的item
{
    int data = intArray[front++];
    if(front == MAX)
    {
        front = 0;
    }
    itemCount--;
    return data;
}

