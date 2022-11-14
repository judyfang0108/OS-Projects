#include "my_info.h"
static void *c_start(struct seq_file *m, loff_t *pos)
{
    *pos = cpumask_next(*pos - 1, cpu_online_mask);
    if ((*pos) < nr_cpu_ids)
        return &cpu_data(*pos);
    return NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
    (*pos)++;
    return c_start(m, pos);
}

static void c_stop(struct seq_file *m, void *v)
{
    //Memory
    printk(KERN_ALERT "MEMORY\n");
    seq_puts(m,"\n===============Memory===============\n");
    struct sysinfo si;
    unsigned long pages[NR_LRU_LISTS];
    int lru;
    si_meminfo(&si);
    for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
        pages[lru] = global_node_page_state(NR_LRU_BASE + lru);
    seq_printf(m,"MemTotal:%lu kb\n",si.totalram);
    seq_printf(m,"MemFree:%lu kb\n",si.freeram);
    seq_printf(m,"Buffers:%lu kb\n",si.bufferram);
    seq_printf(m,"Active:%lu kb\n", pages[LRU_ACTIVE_ANON] +pages[LRU_ACTIVE_FILE]);
    seq_printf(m,"Inactive:%lu kb\n", pages[LRU_INACTIVE_ANON] +pages[LRU_INACTIVE_FILE]);
    seq_printf(m,"Shmem:%lu kb\n",si.sharedram);
    seq_printf(m,"Dirty:%lu kB\n",global_node_page_state(NR_FILE_DIRTY));
    seq_printf(m,"Writeback:%lu kB\n", global_node_page_state(NR_WRITEBACK));
    seq_printf(m,"KernelStack:%lu kB\n",global_zone_page_state(NR_KERNEL_STACK_KB));
    seq_printf(m,"PageTables:%lu kB\n",global_zone_page_state(NR_PAGETABLE));

    //Time info
    printk(KERN_ALERT "Time\n");
    struct timespec uptime;
    struct timespec idle;
    u64 nsec;
    u32 rem;
    int temp;
    nsec = 0;
    for_each_possible_cpu(temp)
    nsec += (__force u64) kcpustat_cpu(temp).cpustat[CPUTIME_IDLE];
    get_monotonic_boottime(&uptime);
    idle.tv_sec = div_u64_rem(nsec, NSEC_PER_SEC, &rem);
    idle.tv_nsec = rem;
    seq_puts(m,"\n===============Time===============\n");
    seq_printf(m, "Uptime: %lu.%02lu (s)\nIdletime: %lu.%02lu (s)\n", (unsigned long) uptime.tv_sec, (uptime.tv_nsec / (NSEC_PER_SEC / 100)), (unsigned long) idle.tv_sec, (idle.tv_nsec / (NSEC_PER_SEC / 100)));
}
static void aperfmperf_snapshot_khz(void *dummy)
{
    u64 aperf, aperf_delta;
    u64 mperf, mperf_delta;
    struct aperfmperf_sample *s = this_cpu_ptr(&samples);
    unsigned long flags;

    local_irq_save(flags);
    rdmsrl(MSR_IA32_APERF, aperf);
    rdmsrl(MSR_IA32_MPERF, mperf);
    local_irq_restore(flags);

    aperf_delta = aperf - s->aperf;
    mperf_delta = mperf - s->mperf;


    // There is no architectural guarantee that MPERF
    // increments faster than we can read it.

    if (mperf_delta == 0)
        return;

    s->time = ktime_get();
    s->aperf = aperf;
    s->mperf = mperf;
    s->khz = div64_u64((cpu_khz * aperf_delta), mperf_delta);
}

static int aperfmperf_snapshot_cpu(int cpu, ktime_t now, bool wait)
{
    s64 time_delta = ktime_ms_delta(now, per_cpu(samples.time, cpu));

    // Don't bother re-computing within the cache threshold time.
    if (time_delta < APERFMPERF_CACHE_THRESHOLD_MS)
        return true;

    smp_call_function_single(cpu, aperfmperf_snapshot_khz, NULL, wait);

    // Return false if the previous iteration was too long ago.
    return time_delta <= APERFMPERF_STALE_THRESHOLD_MS;
}

void arch_freq_prepare_all(void)
{
    ktime_t now = ktime_get();
    bool wait = false;
    int cpu;

    if (!cpu_khz)
        return;

    if (!static_cpu_has(X86_FEATURE_APERFMPERF))
        return;

    for_each_online_cpu(cpu)
    if (!aperfmperf_snapshot_cpu(cpu, now, false))
        wait = true;

    if (wait)
        msleep(APERFMPERF_REFRESH_DELAY_MS);
}

static int proc_show(struct seq_file *m, void *v)
{
    struct cpuinfo_x86 *c = v;
    unsigned int cpu;
    cpu = c->cpu_index;

    if (cpu==0)
    {
        printk(KERN_ALERT "Linux\n");
        seq_puts(m,"\n==============Version===============\n");
        seq_printf(m, "Linux Version: %s %s\n",utsname()->sysname, utsname()->release);
        seq_puts(m,"\n================CPU=================\n");
    }
    //CPU
    printk(KERN_ALERT "CPU\n");
    arch_freq_prepare_all();
    seq_printf(m, "\nprocessor\t: %d\n",cpu);
    seq_printf (m,"model\t\t: %s\n",(c->x86_model_id[0] ? c->x86_model_id : "unknown"));
#ifdef CONFIG_SMP
    seq_printf(m, "physical id\t: %d\n", c->phys_proc_id);
    seq_printf(m, "core id\t\t: %d\n", c->cpu_core_id);
#endif
    // Cache size
    if (c->x86_cache_size >= 0)
        seq_printf(m, "cache size\t: %d KB\n", c->x86_cache_size);
    seq_printf(m, "clflush size\t: %u\n", c->x86_clflush_size);
    seq_printf(m, "cache_alignment\t: %d\n", c->x86_cache_alignment);
    seq_printf(m, "address sizes\t: %u bits physical, %u bits virtual\n",c->x86_phys_bits, c->x86_virt_bits);


    return 0;
}
const struct seq_operations seq_ops =
{
    .start = c_start,
    .next = c_next,
    .stop = c_stop,
    .show = proc_show
};
static int proc_open(struct inode *inode, struct file *file)
{
    return seq_open (file,&seq_ops);
}

static const struct file_operations proc_fops =
{
    .open  = proc_open,
    .read  = seq_read,
    .llseek  = seq_lseek,
    .release = seq_release
};
static int __init proc_init(void)
{
    printk(KERN_ALERT "Hello\n");
    proc_create("my_info", 0, NULL, &proc_fops);
    return 0;
}
static void proc_exit(void)
{
    printk(KERN_ALERT "Goodbye\n");
    remove_proc_entry("my_info", NULL);
}
module_init(proc_init);
module_exit (proc_exit);

