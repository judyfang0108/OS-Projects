#ifndef OS2021_API_H
#define OS2021_API_H

#define STACK_SIZE 8192

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <json-c/json.h>
#include "function_library.h"

//API
int OS2021_ThreadCreate(char *job_name, char *p_function, char *priority, int cancel_mode);
void OS2021_ThreadCancel(char *job_name);
void OS2021_ThreadWaitEvent(int event_id);
void OS2021_ThreadSetEvent(int event_id);
void OS2021_ThreadWaitTime(int msec);
void OS2021_DeallocateThreadResource();
void OS2021_TestCancel();

void CreateContext(ucontext_t *, ucontext_t *, void *);
void ResetTimer();
void Timer_Handler();
void Scheduler();
void Dispatcher();
void StartSchedulingSimulation();
void PrintReport(int signal);

void Parser();

//thread structure
typedef struct data data;
typedef struct thread threads;
typedef struct thread * threads_ptr;


//multilevel feedback queue
void Enqueue(threads_ptr *, threads_ptr *);
threads_ptr deq(threads_ptr *);
threads_ptr create_thread(char *, int, char *, int);
void ChangePriority(threads_ptr *, int, int);

#endif
