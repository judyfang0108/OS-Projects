#include "MemManager.h"

Node*copy_node(char*name,int frame)
{
    Node*temp = malloc(sizeof(Node));
    memset(temp,0,sizeof(Node));
    strcpy(temp->name,name);
    temp->frame = frame;
    temp->next = NULL;
    return temp;
}

void add_node(Node**root,char*name,int frame)
{
    Node*temp = *root;
    if(!temp)
    {
        *root = copy_node(name,frame);
    }
    else
    {
        while(temp->next)
        {
            temp = temp->next;
        }
        temp->next = copy_node(name,frame);
    }
}

void enqueue(FFL**root,FFL*new)
{
    FFL*temp = *root;
    if(!temp)
    {
        *root = new;
    }
    else
    {
        while(temp->next)
        {
            temp = temp->next;
        }
        temp->next = new;
    }
    new->next = NULL;
}

FFL*dequeue(FFL**root)
{
    FFL*temp = *root;
    if(temp)
    {
        *root = temp->next;
        temp->next = NULL;
        return temp;
    }
    return NULL;
}

TLBE*make_TLB()
{

    TLBE*root,*temp;
    root = temp = malloc(sizeof(TLBE));
    memset(temp, 0, sizeof(TLBE));
    for(int i = 1 ; i < 32 ; i++)
    {

        temp->next = malloc(sizeof(TLBE));
        temp = temp->next;
        memset(temp, 0, sizeof(TLBE));

    }
    return root;

}

FFL* make_free_memory_list(int phy_num)
{

    FFL*root,*temp;
    root = temp = malloc(sizeof(FFL));
    temp->frame = 0;
    temp->next = NULL;
    for(int i = 1 ; i < phy_num ; i++)
    {

        temp->next = malloc(sizeof(FFL));
        temp = temp->next;
        temp->frame = i;
        temp->next = NULL;

    }
    return root;
}

void flush_TLB(TLBE**TLB)
{
    TLBE*root = *TLB;
    while(root)
    {
        root->valid = 0;
        root = root->next;
    }
}

int search_TLB(TLBE**TLB,int page,char*policy)
{
    TLBE*pre,*post;
    pre = post= *TLB;

    while(post)
    {

        if(post->valid == 1)
        {

            if(post->VPN == page)
            {


                if(strcmp(policy,"LRU") == 0)
                {
                    if(post != pre)
                    {
                        pre->next = post->next;
                        post->next = *TLB;
                        *TLB = post;
                    }
                }

                return post->PFN;

            }

        }
        pre = post;
        post = post->next;
    }

    return -1;

}

int update_TLB(TLBE**TLB,int page,int frame,char * policy)
{
    TLBE*pre,*post;
    pre = post= *TLB;
    int TLB_full = 1;
    int vpn = 0;
    int x = 0;
    while(post)
    {
        if(post->valid == 0)
        {
            post->valid = 1;
            post->VPN = page;
            post->PFN = frame;
            if(strcmp(policy,"LRU") == 0)
            {
                if(post != pre)
                {
                    pre->next = post->next;
                    post->next = *TLB;
                    *TLB = post;
                }
            }
            TLB_full = 0;
            break;
        }
        pre = post;
        post = post->next;
    }
    if(TLB_full == 1) //TLB滿了要取代
    {
        if(strcmp(policy,"LRU") == 0)
        {
            post = pre = *TLB;
            if(post)
            {
                while(post->next)
                {
                    pre = post;
                    post = post->next;
                }
            }
            pre->next = NULL;
            post->next = *TLB;
            *TLB = post;
            vpn = post->VPN;
            post->PFN = frame;
            post->VPN = page;
        }
        else if(strcmp(policy,"RANDOM") == 0)
        {
            srand(time(NULL));
            x = rand() % 32;
            post = *TLB;
            while(x)
            {
                post = post->next;
                x--;
            }
            vpn = post->VPN;
            post->VPN = page;
            post->PFN = frame;
        }
        return vpn;

    }
    else//TLB還有位置
        return -1;
}

void invalid_TLB(TLBE**TLB,int page)
{
    TLBE*post= *TLB;
    while(post)
    {
        if(post->valid == 1)
        {
            if(post->VPN == page)
                post->valid=0;
        }
        post = post->next;
    }
}

char TLB_policy[10];
char page_policy[10];
char frame_policy[10];
int process_num = 0;
int vir_num = 0;
int phy_num = 0;
void read_sys_config()
{
    char line[6][50];
    for (int i=0; i<6; i++)
        for (int j=0; j<50; j++)
            line[i][j]=0;

    FILE *fp = fopen("sys_config1.txt", "r");
    int line_count=0;
    while(fgets(line[line_count],sizeof(line[line_count]),fp)!=NULL)
    {
        line_count++;
    }
    //read file
    sscanf(line[0],"TLB Replacement Policy: %s",TLB_policy);
    printf ("tlb policy: %s\n",TLB_policy);

    sscanf(line[1],"Page Replacement Policy: %s",page_policy);
    printf ("page policy: %s\n",page_policy);

    sscanf(line[2],"Frame Allocation Policy: %s",frame_policy);
    printf ("frame_policy: %s\n",frame_policy);

    char process[10];
    sscanf(line[3],"Number of Processes: %s",process);
    process_num = atoi(process);
    printf ("num of process: %d\n",process_num);

    char VP[10];
    sscanf(line[4],"Number of Virtual Page: %s",VP);
    vir_num = atoi(VP);
    printf ("virtual page: %d\n",vir_num);

    char PF[10];
    sscanf(line[5],"Number of Physical Frame: %s",PF);
    phy_num = atoi(PF);
    printf ("physical_frame: %d\n",phy_num);
    fclose(fp);
}

Node* read_trace()
{
    FILE*trace = fopen("trace.txt", "r");
    char word;
    char name[10];
    char frame[10];
    int namecount = 0;
    int framecount = 0;
    int flag1 = 0;
    int flag2 = 0;
    Node*root = NULL;
    memset(name, 0, sizeof(name));
    while(word = getc(trace))
    {
        if(word == '\n' || word == EOF)
        {
            add_node(&root,name,atoi(frame));
            namecount = 0;
            framecount = 0;
            memset(name, 0, sizeof(name));
            memset(frame, 0, sizeof(frame));
            if(word == EOF)
            {
                break;
            }
            continue;
        }
        if(word == '(')
        {
            flag1 = !flag1;
            continue;
        }
        if(word == ',')
        {
            flag1 = !flag1;
            flag2 = !flag2;
            continue;
        }
        if(word == ')')
        {
            flag2 = !flag2;
            continue;
        }
        if(flag1)
        {
            name[namecount++] = word;
        }
        if(flag2)
        {
            frame[framecount++] = word;
        }
    }
    fclose(trace);
    return root;
}

int main()
{
    read_sys_config();
    char cur_process = 0;
    int time_counter = 0;
    int block_counter = 0;
    int flag = 0;

    TLBE*TLB = make_TLB();
    PTE vir[process_num][vir_num];
    FFL* free_memory_list = make_free_memory_list(phy_num);
    FFL* global_victim_page = NULL;
    FFL* local_victim_page[process_num];
    int hit_num[process_num];
    int ref_num[process_num];
    int pagefault_num[process_num];
    memset(vir,0,sizeof(vir));
    memset(hit_num,0,sizeof(hit_num));
    memset(ref_num,0,sizeof(ref_num));
    memset(pagefault_num,0,sizeof(pagefault_num));

    for(int i = 0 ; i < process_num ; i++)
    {
        local_victim_page[i] = NULL;
        for(int j = 0 ; j < vir_num ; j++)
        {
            vir[i][j].dbi = -1;
        }
    }

    Node*root = read_trace();
    flush_TLB(&TLB);
    FILE *trace_output;
    trace_output = fopen("trace_output.txt","w");

    while(root)
    {
        PTE*page_table = vir[root->name[0] - 'A'];
        FFL*temp = NULL;
        int page = root->frame;
        int frame ;
        int evict_page;
        int dest;
        int cur_process_idx = 0;
        int temp_process_idx = 0;
        int temp_page = 0;
        char evict_process;
        time_counter++;
        if(cur_process != root->name[0])
        {
            flush_TLB(&TLB);
        }

        cur_process = root->name[0];
        cur_process_idx = cur_process - 'A';

        ref_num[cur_process_idx]++;

        //TLB miss
        if((frame = search_TLB(&TLB,page,TLB_policy)) == -1)
        {
            //page_ref_num[cur_process_idx]++;
            //page Hit
            if(page_table[page].valid == 1 && page_table[page].present == 1)
            {
                frame = page_table[page].frame;
                page_table[page].reference = 1;
                fprintf(trace_output,"Process %c, TLB Miss, Page Hit, %d=>%d\n",cur_process,page,frame);
            }
            //page fault casue by invalid
            else if(page_table[page].valid == 0)
            {
                if(free_memory_list)
                {
                    temp = dequeue(&free_memory_list);
                    //global victim page
                    temp->next = NULL;
                    if(strcmp(frame_policy,"GLOBAL") == 0)
                    {
                        enqueue(&global_victim_page,temp);
                    }
                    else if(strcmp(frame_policy,"LOCAL") == 0)
                    {
                        enqueue(&(local_victim_page[cur_process_idx]),temp);
                    }
                    //local victim page
                    frame = temp->frame;
                    temp->page = page;
                    temp->process = cur_process;

                    page_table[page].valid = 1;
                    page_table[page].reference = 1;
                    page_table[page].present = 1;
                    page_table[page].frame = frame;
                    fprintf(trace_output,"Process %c, TLB Miss, Page Fault, %d, Evict -1 of Process %c to -1, %d<<-1\n",cur_process,frame,cur_process,page);
                    pagefault_num[cur_process_idx]++;
                }
                else
                {
                    if(flag == 0)
                    {
                        if(strcmp(frame_policy,"GLOBAL") == 0)
                        {

                            temp = global_victim_page;
                            if(temp)
                            {
                                while(temp->next)
                                {
                                    temp = temp->next;
                                }
                            }
                            temp->next = global_victim_page;


                        }
                        else if(strcmp(frame_policy,"LOCAL") == 0)
                        {

                            for(int i = 0 ; i < process_num ; i++)
                            {

                                temp = local_victim_page[i];
                                if(temp)
                                {
                                    while(temp->next)
                                    {
                                        temp = temp->next;
                                    }
                                }
                                temp->next = local_victim_page[i];

                            }

                        }

                    }
                    flag = 1;
                    if(strcmp(page_policy,"FIFO") == 0)
                    {

                        if(strcmp(frame_policy,"GLOBAL") == 0)
                        {

                            temp = global_victim_page;
                            global_victim_page = global_victim_page->next;

                            temp_process_idx = temp->process - 'A';
                            temp_page = temp->page;

                            frame = temp->frame;

                            vir[temp_process_idx][temp_page].present = 0;
                            vir[temp_process_idx][temp_page].reference = 0;

                            if(itemCount==0)//disk is empty
                                vir[temp_process_idx][temp_page].dbi = block_counter++;

                            if(itemCount!=0)
                            {
                                int num=removeData();
                                if(num==-1)
                                    vir[temp_process_idx][temp_page].dbi = block_counter++;
                                else
                                    vir[temp_process_idx][temp_page].dbi=num;
                            }

                            evict_page = temp_page;
                            evict_process = temp->process;
                            dest = vir[temp_process_idx][temp_page].dbi;

                            if(temp->process == cur_process)
                                invalid_TLB(&TLB,page);

                            temp->process = cur_process;
                            temp->page = page;
                            page_table[page].valid = 1;
                            page_table[page].reference = 1;
                            page_table[page].present = 1;
                            page_table[page].frame = frame;
                            fprintf(trace_output,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<%d\n",cur_process,frame,evict_page,evict_process,dest,page,page_table[page].dbi);
                            insert(page_table[page].dbi);
                            pagefault_num[cur_process_idx]++;
                        }

                        else if(strcmp(frame_policy,"LOCAL") == 0)
                        {

                            temp = local_victim_page[cur_process_idx];
                            local_victim_page[cur_process_idx] = local_victim_page[cur_process_idx]->next;

                            temp_process_idx = temp->process - 'A';
                            temp_page = temp->page;

                            frame = temp->frame;

                            vir[temp_process_idx][temp_page].present = 0;
                            vir[temp_process_idx][temp_page].reference = 0;


                            if(itemCount==0)//disk is empty
                                vir[temp_process_idx][temp_page].dbi = block_counter++;

                            if(itemCount!=0)
                            {
                                int num=removeData();
                                if(num==-1)
                                    vir[temp_process_idx][temp_page].dbi = block_counter++;
                                else
                                    vir[temp_process_idx][temp_page].dbi=num;
                            }

                            evict_page = temp_page;
                            evict_process = temp->process;
                            dest = vir[temp_process_idx][temp_page].dbi;

                            if(temp->process == cur_process)
                                invalid_TLB(&TLB,evict_page );

                            temp->process = cur_process;
                            temp->page = page;

                            page_table[page].valid = 1;
                            page_table[page].reference = 1;
                            page_table[page].present = 1;
                            page_table[page].frame = frame;
                            fprintf(trace_output,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<-1\n",cur_process,frame,evict_page,evict_process,dest,page);
                            insert(page_table[page].dbi);
                            pagefault_num[cur_process_idx]++;
                        }

                    }
                    else if(strcmp(page_policy,"CLOCK") == 0)
                    {

                        if(strcmp(frame_policy,"GLOBAL") == 0)
                        {

                            while(vir[global_victim_page->process - 'A'][global_victim_page->page].reference)
                            {

                                vir[global_victim_page->process - 'A'][global_victim_page->page].reference = 0;
                                global_victim_page = global_victim_page->next;

                            }
                            temp = global_victim_page;
                            global_victim_page = global_victim_page->next;

                            temp_process_idx = temp->process - 'A';
                            temp_page = temp->page;

                            frame = temp->frame;

                            vir[temp_process_idx][temp_page].present = 0;
                            vir[temp_process_idx][temp_page].reference = 0;

                            if(itemCount==0) //disk is empty
                            {
                                vir[temp_process_idx][temp_page].dbi = block_counter++;
                            }
                            if(itemCount!=0)
                            {
                                int num=removeData();
                                if(num==-1)
                                    vir[temp_process_idx][temp_page].dbi = block_counter++;
                                else
                                    vir[temp_process_idx][temp_page].dbi=num;
                            }

                            evict_page = temp_page;
                            evict_process = temp->process;
                            dest = vir[temp_process_idx][temp_page].dbi;

                            if(temp->process == cur_process)
                            {

                                invalid_TLB(&TLB,page);

                            }

                            temp->process = cur_process;
                            temp->page = page;

                            page_table[page].valid = 1;
                            page_table[page].reference = 1;
                            page_table[page].present = 1;
                            page_table[page].frame = frame;
                            fprintf(trace_output,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<-1\n",cur_process,frame,evict_page,evict_process,dest,page);
                            insert(page_table[page].dbi);
                            pagefault_num[cur_process_idx]++;
                        }

                        else if(strcmp(frame_policy,"LOCAL") == 0)
                        {

                            while(vir[local_victim_page[cur_process_idx]->process - 'A'][local_victim_page[cur_process_idx]->page].reference)
                            {

                                vir[local_victim_page[cur_process_idx]->process - 'A'][local_victim_page[cur_process_idx]->page].reference = 0;
                                local_victim_page[cur_process_idx] = local_victim_page[cur_process_idx]->next;

                            }
                            temp = local_victim_page[cur_process_idx];
                            local_victim_page[cur_process_idx] = local_victim_page[cur_process_idx]->next;

                            temp_process_idx = temp->process - 'A';
                            temp_page = temp->page;

                            frame = temp->frame;

                            vir[temp_process_idx][temp_page].present = 0;
                            vir[temp_process_idx][temp_page].reference = 0;

                            if(itemCount==0) //disk is empty
                            {
                                vir[temp_process_idx][temp_page].dbi = block_counter++;
                            }
                            if(itemCount!=0)
                            {
                                int num=removeData();
                                if(num==-1)
                                    vir[temp_process_idx][temp_page].dbi = block_counter++;
                                else
                                    vir[temp_process_idx][temp_page].dbi=num;
                            }
                            evict_page = temp_page;
                            evict_process = temp->process;
                            dest = vir[temp_process_idx][temp_page].dbi;

                            if(temp->process == cur_process)
                            {

                                invalid_TLB(&TLB,evict_page);

                            }

                            temp->process = cur_process;
                            temp->page = page;

                            page_table[page].valid = 1;
                            page_table[page].reference = 1;
                            page_table[page].present = 1;
                            page_table[page].frame = frame;
                            fprintf(trace_output,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<-1\n",cur_process,frame,evict_page,evict_process,dest,page);
                            insert(page_table[page].dbi);
                            pagefault_num[cur_process_idx]++;
                        }


                    }

                }
            }
            //page fault casue by page in disk
            else if(page_table[page].present == 0)
            {
                if(strcmp(page_policy,"FIFO") == 0)
                {
                    if(strcmp(frame_policy,"GLOBAL") == 0)
                    {

                        temp = global_victim_page;
                        global_victim_page = global_victim_page->next;

                        temp_process_idx = temp->process - 'A';
                        temp_page = temp->page;

                        frame = temp->frame;

                        vir[temp_process_idx][temp_page].present = 0;
                        vir[temp_process_idx][temp_page].reference = 0;

                        if(itemCount!=0)
                        {
                            int num=removeData();
                            if(num==-1)
                                vir[temp_process_idx][temp_page].dbi = block_counter++;
                            else
                                vir[temp_process_idx][temp_page].dbi=num;
                        }

                        evict_page = temp_page;
                        evict_process = temp->process;
                        dest = vir[temp_process_idx][temp_page].dbi;

                        if(temp->process == cur_process)
                        {

                            invalid_TLB(&TLB,page);

                        }

                        temp->process = cur_process;
                        temp->page = page;

                        page_table[page].valid = 1;
                        page_table[page].reference = 1;
                        page_table[page].present = 1;
                        page_table[page].frame = frame;
                        fprintf(trace_output,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<%d\n",cur_process,frame,evict_page,evict_process,dest,page,page_table[page].dbi);
                        insert(page_table[page].dbi);
                        pagefault_num[cur_process_idx]++;

                    }

                    else if(strcmp(frame_policy,"LOCAL") == 0)
                    {

                        temp = local_victim_page[cur_process_idx];
                        local_victim_page[cur_process_idx] = local_victim_page[cur_process_idx]->next;

                        temp_process_idx = temp->process - 'A';
                        temp_page = temp->page;

                        frame = temp->frame;

                        vir[temp_process_idx][temp_page].present = 0;
                        vir[temp_process_idx][temp_page].reference = 0;

                        if(itemCount==0) //disk is empty
                        {
                            vir[temp_process_idx][temp_page].dbi = block_counter++;
                        }
                        if(itemCount!=0)
                        {
                            int num=removeData();
                            if(num==-1)
                                vir[temp_process_idx][temp_page].dbi = block_counter++;
                            else
                                vir[temp_process_idx][temp_page].dbi=num;
                        }

                        evict_page = temp_page;
                        evict_process = temp->process;
                        dest = vir[temp_process_idx][temp_page].dbi;

                        if(temp->process == cur_process)
                        {

                            invalid_TLB(&TLB,evict_page);

                        }

                        temp->process = cur_process;
                        temp->page = page;

                        page_table[page].valid = 1;
                        page_table[page].reference = 1;
                        page_table[page].present = 1;
                        page_table[page].frame = frame;
                        fprintf(trace_output,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<%d\n",cur_process,frame,evict_page,evict_process,dest,page,page_table[page].dbi);
                        insert(page_table[page].dbi);
                        pagefault_num[cur_process_idx]++;
                    }
                }
                else if(strcmp(page_policy,"CLOCK") == 0)
                {
                    if(strcmp(frame_policy,"GLOBAL") == 0)
                    {

                        while(vir[global_victim_page->process - 'A'][global_victim_page->page].reference)
                        {

                            vir[global_victim_page->process - 'A'][global_victim_page->page].reference = 0;
                            global_victim_page = global_victim_page->next;

                        }
                        temp = global_victim_page;
                        global_victim_page = global_victim_page->next;

                        temp_process_idx = temp->process - 'A';
                        temp_page = temp->page;

                        frame = temp->frame;

                        vir[temp_process_idx][temp_page].present = 0;
                        vir[temp_process_idx][temp_page].reference = 0;
                        if(itemCount==0) //disk is empty
                        {
                            vir[temp_process_idx][temp_page].dbi = block_counter++;
                        }
                        if(itemCount!=0)
                        {
                            int num=removeData();
                            if(num==-1)
                                vir[temp_process_idx][temp_page].dbi = block_counter++;
                            else
                                vir[temp_process_idx][temp_page].dbi=num;
                        }

                        evict_page = temp_page;
                        evict_process = temp->process;
                        dest = vir[temp_process_idx][temp_page].dbi;

                        if(temp->process == cur_process)
                        {

                            invalid_TLB(&TLB,page);

                        }

                        temp->process = cur_process;
                        temp->page = page;

                        page_table[page].valid = 1;
                        page_table[page].reference = 1;
                        page_table[page].present = 1;
                        page_table[page].frame = frame;
                        fprintf(trace_output,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<%d\n",cur_process,frame,evict_page,evict_process,dest,page,page_table[page].dbi);
                        insert(page_table[page].dbi);
                        pagefault_num[cur_process_idx]++;
                    }

                    else if(strcmp(frame_policy,"LOCAL") == 0)
                    {
                        while(vir[local_victim_page[cur_process_idx]->process - 'A'][local_victim_page[cur_process_idx]->page].reference)
                        {
                            vir[local_victim_page[cur_process_idx]->process - 'A'][local_victim_page[cur_process_idx]->page].reference = 0;
                            local_victim_page[cur_process_idx] = local_victim_page[cur_process_idx]->next;

                        }
                        temp = local_victim_page[cur_process_idx];
                        local_victim_page[cur_process_idx] = local_victim_page[cur_process_idx]->next;

                        temp_process_idx = temp->process - 'A';
                        temp_page = temp->page;

                        frame = temp->frame;

                        vir[temp_process_idx][temp_page].present = 0;
                        vir[temp_process_idx][temp_page].reference = 0;

                        if(itemCount==0) //disk is empty
                        {
                            vir[temp_process_idx][temp_page].dbi = block_counter++;
                        }
                        if(itemCount!=0)
                        {
                            int num=removeData();
                            if(num==-1)
                                vir[temp_process_idx][temp_page].dbi = block_counter++;
                            else
                                vir[temp_process_idx][temp_page].dbi=num;
                        }

                        evict_page = temp_page;
                        evict_process = temp->process;
                        dest = vir[temp_process_idx][temp_page].dbi;

                        if(temp->process == cur_process)
                            invalid_TLB(&TLB,evict_page);


                        temp->process = cur_process;
                        temp->page = page;

                        page_table[page].valid = 1;
                        page_table[page].reference = 1;
                        page_table[page].present = 1;
                        page_table[page].frame = frame;
                        fprintf(trace_output,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<%d\n",cur_process,frame,evict_page,evict_process,dest,page,page_table[page].dbi);
                        insert(page_table[page].dbi);
                        pagefault_num[cur_process_idx]++;

                    }

                }
            }
            update_TLB(&TLB,page,frame,TLB_policy);
        }
        //TLB hit
        else
        {
            hit_num[cur_process_idx]++;//每一次trace，經過TLB更新後，都會hit到一次，因此可以知道每一個process被trace了幾次
            fprintf(trace_output,"Process %c, TLB Hit, %d=>%d\n",root->name[0],page,frame);
            page_table[page].reference = 1;
            root = root->next;
        }
    }
    fclose(trace_output);

    FILE *analysis = fopen("analysis.txt","w");
    for(int i = 0 ; i < process_num ; i++)
    {
        double hit_rate = ((double)hit_num[i])/((double)ref_num[i]);
        double pagefault_rate = ((double)pagefault_num[i])/((double)hit_num[i]);
        fprintf(analysis,"Process %c, Effective Access Time = %.3f\n",i+'A',(hit_rate*120 + (1-hit_rate)*220));
        if(i==process_num-1)//avoid print new line at the end
            fprintf(analysis,"Process %c, Page Fault Rate: %.3f",i+'A',pagefault_rate);
        else
            fprintf(analysis,"Process %c, Page Fault Rate: %.3f\n",i+'A',pagefault_rate);

    }
    fclose(analysis);

    return 0;

}

