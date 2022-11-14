#include "os2021_thread_api.h"

//Define thread
struct data
{
    char *name;
    int id;
    //priority: H=2, M=1, L=0
    int b_priority;//initial priority
    int c_priority;//current priority
    int mode;//0=asynchronous, 1=deffer(reclaimer can't goto the end)
    int status;//cancel status:0=nothing, 1=waiting cancel
    int event;//event I want wait(0~7), -1 = not waiting
    long queuing;//queuing time
    long waiting;//waiting time
    long total;//how long you need to wait
    long counting;//use when ThreadWaitTime
    ucontext_t content;//context record
};

struct thread
{
    data data;
    threads_ptr next;
};

struct itimerval Signaltimer;
ucontext_t dispatch_context;//dispatcher context
ucontext_t timer_context;//if function will goto end
int num = 0;//total thread's id
long quantum = 0;//time quantum

//priority feedback queue head
threads_ptr running = NULL;
threads_ptr ready = NULL;
threads_ptr waited = NULL;
threads_ptr terminate = NULL;

//priority parameter
char priority_arr[3] = {'L', 'M', 'H'};
int value[3] = {300, 200, 100};//time quantum value, value[0] = low priority's time quantum

int OS2021_ThreadCreate(char *job_name, char *p_function, char *priority, int cancel_mode)
{
    threads_ptr new_th = create_thread(job_name, num, priority, cancel_mode);
    num++;

    if(strcmp(p_function, "Function1") == 0)
        CreateContext(&(new_th->data.content), &timer_context, &Function1);
    else if(strcmp(p_function, "Function2") == 0)
        CreateContext(&(new_th->data.content), &timer_context, &Function2);
    else if(strcmp(p_function, "Function3") == 0)
        CreateContext(&(new_th->data.content), &timer_context, &Function3);
    else if(strcmp(p_function, "Function4") == 0)
        CreateContext(&(new_th->data.content), &timer_context, &Function4);
    else if(strcmp(p_function, "Function5") == 0)
        CreateContext(&(new_th->data.content), &timer_context, &Function5);
    else if(strcmp(p_function, "ResourceReclaim") == 0)
        CreateContext(&(new_th->data.content), NULL, &ResourceReclaim);
    else
    {
        free(new_th);
        return -1;
    }

    Enqueue(&new_th, &ready);
    return new_th->data.id;
}

void OS2021_ThreadCancel(char *job_name)
{
    threads_ptr target = NULL;
    threads_ptr temp_th = ready;
    threads_ptr ex_th = NULL;
    if(strcmp("reclaimer", job_name)==0)
        return;//reclaimer can't enter terminate state

    /*tried to find target in ready queue or wait queue */
    while (temp_th!=NULL)
    {
        if(strcmp(temp_th->data.name, job_name)==0)
        {
            target = temp_th;
            break;
        }
        else
        {
            ex_th = temp_th;
            temp_th = temp_th->next;
        }
    }
    if(target == NULL)
    {
        temp_th = waited;
        ex_th = NULL;
        while(temp_th != NULL)
        {
            if(strcmp(temp_th->data.name, job_name) == 0)
            {
                target = temp_th;
                break;
            }
            else
            {
                ex_th = temp_th;
                temp_th = temp_th->next;
            }
        }
    }
    /*if find target, move it to terminate queue or change state*/
    if(target != NULL)
    {
        if(target->data.mode == 0)
        {
            /*dequeue from original queue*/
            if(target == waited)
                waited = target->next;
            else
                ex_th->next = target->next;
            Enqueue(&target, &terminate);//Enqueueueue to terminate queue
        }
        else
        {
            target->data.status = 1;//change cancel state but not goto terminate now, wait for cancel point
            printf("%s wants to cancel thread %s\n", running->data.name, target->data.name);
        }
    }
    return;
}

void OS2021_ThreadWaitEvent(int event_id)
{
    threads *target = running;
    target->data.event = event_id;
    printf("%s wants to waiting for event %d\n", target->data.name, event_id);
    ChangePriority(&target, quantum, value[target->data.c_priority]);//change priority
    Enqueue(&target, &waited);//change to wait queue
    swapcontext(&(target->data.content), &dispatch_context);//save current data.status and reschedule
    return;
}

void OS2021_ThreadSetEvent(int event_id)
{
    threads_ptr bullet_th = running;
    threads_ptr hit_th = NULL;
    threads_ptr temp_th = waited;
    threads_ptr ex_th = NULL;
    //try to find target
    while(temp_th != NULL)
    {
        if(temp_th->data.event != event_id)
        {
            ex_th = temp_th;
            temp_th = temp_th->next;
        }
        else
        {
            hit_th = temp_th;
            hit_th->data.event = -1;
            if(hit_th == waited)
                waited = waited->next;
            else
            {
                ex_th->next = hit_th->next;
                hit_th->next = NULL;
            }
            printf("%s changes the data.status of %s to READY.\n", bullet_th->data.name, hit_th->data.name);
            Enqueue(&hit_th, &ready);
            return;
        }
    }
    return;
}

void OS2021_ThreadWaitTime(int msec)
{
    threads *target = running;
    ChangePriority(&target, quantum, value[target->data.c_priority]);
    target->data.total = msec;
    target->next = NULL;
    Enqueue(&target, &waited);
    swapcontext(&(target->data.content), &dispatch_context);//save current data.status and reschedule
    return;
}

void OS2021_DeallocateThreadResource()
{
    threads_ptr target = terminate;
    while(target != NULL)
    {
        printf("The memory space by %s has been released.\n", target->data.name);
        terminate = terminate->next;
        free(target);
        target = terminate;
    }
    return;
}

void OS2021_TestCancel()
{
    threads *target = running;
    if(target->data.status == 1)//change when it was cancel by somebody
    {
        Enqueue(&target, &terminate);//change to terminate state
        setcontext(&dispatch_context);//reschedule
    }
    else
        return;
}

void CreateContext(ucontext_t *context, ucontext_t *next_context, void *func)
{
    getcontext(context);//initialize context
    context->uc_stack.ss_sp = malloc(STACK_SIZE);
    context->uc_stack.ss_size = STACK_SIZE;
    context->uc_stack.ss_flags = 0;
    context->uc_link = next_context;//next context
    makecontext(context,(void (*)(void))func,0);//link to which function
}

void ResetTimer()
{
    Signaltimer.it_value.tv_sec = 0;
    Signaltimer.it_value.tv_usec = 10000;
    if(setitimer(ITIMER_REAL, &Signaltimer, NULL) < 0)
    {
        printf("ERROR SETTING TIME SIGALRM!\n");
    }
}

void TimerHandler()
{
    quantum += 10;
    //calculate related time
    threads_ptr temp_th = ready;
    threads_ptr ex_th = NULL;
    while(temp_th != NULL)
    {
        temp_th->data.queuing += 10;
        temp_th = temp_th->next;
    }
    // add wait time
    temp_th = waited;
    while(temp_th != NULL)
    {
        temp_th->data.waiting += 10;
        if(temp_th->data.total != 0)
        {
            threads_ptr target = temp_th;
            threads_ptr target_ex = ex_th;
            target->data.counting ++;
            if(target->data.counting >= target->data.total)
            {
                target->data.total = 0;
                target->data.counting = 0;
                if(target == waited)
                    waited = waited->next;
                else
                    target_ex->next = target->next;
                Enqueue(&target, &ready);
            }
        }
        ex_th = temp_th;
        temp_th = temp_th->next;
    }
    //if time excess time quantum, change another thread
    if(quantum >= value[running->data.c_priority])
    {
        //change priority
        if(running->data.c_priority !=0)
        {
            running->data.c_priority--;
            printf("The priority of thread %s is changed from %c to %c\n", running->data.name, priority_arr[running->data.c_priority+1], priority_arr[running->data.c_priority]);
        }
        Enqueue(&running, &ready);//send it to ready queue
        swapcontext(&(running->data.content), &dispatch_context);//reschedule
    }
    //if not execeed, keep going
    ResetTimer();
    return;
}

void PrintReport(int signal)
{
    printf("\n");
    printf("**************************************************************************************************\n");
    printf("*\tTID\tName\t\tState\t\tB_Priority\tC_Priority\tQ_Time\tW_time\t *\n");
    printf("*\t%d\t%-10s\tRUNNING\t\t%c\t\t%c\t\t%ld\t%ld\t *\n",
           running->data.id, running->data.name, priority_arr[running->data.b_priority], priority_arr[running->data.c_priority], running->data.queuing, running->data.waiting);
    threads_ptr temp_th = ready;
    while(temp_th!=NULL)
    {
        printf("*\t%d\t%-10s\tREADY\t\t%c\t\t%c\t\t%ld\t%ld\t *\n",
               temp_th->data.id, temp_th->data.name, priority_arr[temp_th->data.b_priority], priority_arr[temp_th->data.c_priority], temp_th->data.queuing, temp_th->data.waiting);
        temp_th = temp_th->next;
    }
    temp_th = waited;
    while(temp_th!=NULL)
    {
        printf("*\t%d\t%-10s\tWAITING\t\t%c\t\t%c\t\t%ld\t%ld\t *\n",
               temp_th->data.id, temp_th->data.name, priority_arr[temp_th->data.b_priority], priority_arr[temp_th->data.c_priority], temp_th->data.queuing, temp_th->data.waiting);
        temp_th = temp_th->next;
    }
    printf("**************************************************************************************************\n");
    return;
}

void Scheduler()
{
    running = deq(&ready);
}

void Dispatcher()
{
    running = NULL;//current running thread
    Scheduler();//dequeue ready queue
    quantum = 0;
    ResetTimer();//already pass 10ms,set to init
    setcontext(&(running->data.content));//get running's function context
}

void FinishThread()
{
    threads_ptr target = running;
    running = NULL;
    Enqueue(&target, &terminate);//change from running to terminate state
    setcontext(&dispatch_context);//reschedule
}

void StartSchedulingSimulation()
{
    /*Set Timer*/
    Signaltimer.it_interval.tv_usec = 100;
    Signaltimer.it_interval.tv_sec = 0;
    signal(SIGALRM, TimerHandler);//notify each 10ms
    signal(SIGTSTP, PrintReport);//print PrintReport
    /*Create Context*/
    CreateContext(&dispatch_context, NULL, &Dispatcher);
    CreateContext(&timer_context, &dispatch_context, &FinishThread);
    /*Create thread*/
    OS2021_ThreadCreate("reclaimer", "ResourceReclaim", "L", 1);//create reclaimer thread
    Parser();//create json file's thread
    /*Start Scheduling*/
    setcontext(&dispatch_context);//call diapacthcher
}

void Parser()
{
    struct json_object *parsed_json;//parse all thread info to this object
    //thread object
    struct json_object *thread;
    struct json_object *name;
    struct json_object *entry;
    struct json_object *priority;
    struct json_object *cancel;
    size_t n_thread;//size of Threads
    size_t i;
    int q;//entry function error handler

    parsed_json = json_object_from_file("init_threads.json");
    json_object_object_get_ex(parsed_json, "Threads", &parsed_json);

    n_thread = json_object_array_length(parsed_json);

    //create thread one by one
    for(i=0; i<n_thread; i++)
    {
        thread = json_object_array_get_idx(parsed_json, i);
        json_object_object_get_ex(thread, "name", &name);
        json_object_object_get_ex(thread, "entry function", &entry);
        json_object_object_get_ex(thread, "priority", &priority);
        json_object_object_get_ex(thread, "cancel mode", &cancel);

        //Create thread structure and Enqueueueue
        q = OS2021_ThreadCreate((char*)json_object_get_string(name), (char*)json_object_get_string(entry), (char*)json_object_get_string(priority), json_object_get_int(cancel));
        if(q == -1)
            printf("Incorrect entry function.\n");
    }

    return;
}

threads_ptr create_thread(char *job_name, int id, char *priority, int cancel_mode)
{
    //change priority into integer
    int p;
    if(priority[0] == 'H')
        p = 2;
    else if (priority[0] == 'M')
        p = 1;
    else
        p = 0;

    threads *new_th = malloc(sizeof(threads));
    new_th->data.name = job_name;
    new_th->data.id = id;//data.id = num(original), num = num+1
    new_th->data.b_priority = new_th->data.c_priority = p;
    new_th->data.mode = cancel_mode;
    new_th->data.status = 0;
    new_th->data.event = -1;
    new_th->data.queuing = 0;
    new_th->data.waiting = 0;
    new_th->data.total = 0;
    new_th->data.counting = 0;
    new_th->next = NULL;
    return new_th;
}

//queue: H->H->...M->M->M...->L, previous H is earlier
void Enqueue(threads_ptr *new_th, threads_ptr *head)
{
    threads_ptr temp = (*head);
    threads_ptr temp_ex = NULL;
    if(temp!=NULL)
    {
        while(temp != NULL)
        {
            if(temp->data.c_priority >= (*new_th)->data.c_priority)
            {
                temp_ex = temp;
                temp = temp->next;
            }
            else
            {
                (*new_th)->next = temp;
                if(temp != (*head))
                    temp_ex->next = (*new_th);
                else
                    (*head) = (*new_th);
                break;
            }
        }
        if(temp == NULL)
            temp_ex ->next = (*new_th);
    }
    else//empty queue
    {
        (*head) = (*new_th);
        (*new_th)->next = NULL;
    }
    return;
}

threads_ptr deq(threads_ptr *head)
{
    if((*head) == NULL)
        return NULL;
    else
    {
        threads_ptr leave = (*head);
        (*head) = (*head)->next;
        leave->next = NULL;
        return leave;
    }
}

void ChangePriority(threads_ptr *target, int quantum, int value)
{
    //change priority
    char c[3] = {'L', 'M', 'H'};
    if(quantum < value)
    {
        if((*target)->data.c_priority != 2)
        {
            (*target)->data.c_priority++;
            printf("The priority of thread %s is changed from %c to %c\n",
                   (*target)->data.name, c[(*target)->data.c_priority-1], c[(*target)->data.c_priority]);
        }
    }
    else
    {
        if((*target)->data.c_priority != 0)
        {
            (*target)->data.c_priority--;
            printf("The priority of thread %s is changed from %c to %c\n",
                   (*target)->data.name, c[(*target)->data.c_priority+1], c[(*target)->data.c_priority]);
        }
    }
    return;
}
