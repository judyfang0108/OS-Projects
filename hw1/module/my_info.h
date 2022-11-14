#ifndef my_info
#define my_info
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/utsname.h>
#include <generated/compile.h>
#include <linux/export.h>
#include <linux/uts.h>
#include <linux/version.h>
#include <linux/proc_ns.h>
#include <linux/cpufreq.h>
#include <linux/smp.h>
#include <linux/timex.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/hugetlb.h>
#include <linux/mman.h>
#include <linux/mmzone.h>
#include <linux/quicklist.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <linux/atomic.h>
#include <linux/vmalloc.h>
#ifdef CONFIG_CMA
#include <linux/cma.h>
#endif
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/processor.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/kernel_stat.h>
#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/math64.h>
#include <linux/percpu.h>
#include <linux/smp.h>


struct aperfmperf_sample
{
    unsigned int khz;
    ktime_t time;
    u64 aperf;
    u64 mperf;
};

static DEFINE_PER_CPU(struct aperfmperf_sample, samples);

#define APERFMPERF_CACHE_THRESHOLD_MS 10
#define APERFMPERF_REFRESH_DELAY_MS 10
#define APERFMPERF_STALE_THRESHOLD_MS 1000

// CPU.h
#ifndef ARCH_X86_CPU_H
#define ARCH_X86_CPU_H

/* attempt to consolidate cpu attributes */
struct cpu_dev
{
    const char *c_vendor;

    /* some have two possibilities for cpuid string */
    const char *c_ident[2];

    void            (*c_early_init)(struct cpuinfo_x86 *);
    void  (*c_bsp_init)(struct cpuinfo_x86 *);
    void  (*c_init)(struct cpuinfo_x86 *);
    void  (*c_identify)(struct cpuinfo_x86 *);
    void  (*c_detect_tlb)(struct cpuinfo_x86 *);
    void  (*c_bsp_resume)(struct cpuinfo_x86 *);
    int  c_x86_vendor;
#ifdef CONFIG_X86_32
    /* Optional vendor specific routine to obtain the cache size. */
    unsigned int (*legacy_cache_size)(struct cpuinfo_x86 *,
                                      unsigned int);

    /* Family/stepping-based lookup table for model names. */
    struct legacy_cpu_model_info
    {
        int  family;
        const char *model_names[16];
    }  legacy_models[5];
#endif
};

struct _tlb_table
{
    unsigned char descriptor;
    char tlb_type;
    unsigned int entries;
    /* unsigned int ways; */
    char info[128];
};

#define cpu_dev_register(cpu_devX) \
 static const struct cpu_dev *const __cpu_dev_##cpu_devX __used \
 __attribute__((__section__(".x86_cpu_dev.init"))) = \
 &cpu_devX;

extern const struct cpu_dev *const __x86_cpu_dev_start[],
           *const __x86_cpu_dev_end[];

extern void get_cpu_cap(struct cpuinfo_x86 *c);
extern void cpu_detect_cache_sizes(struct cpuinfo_x86 *c);

unsigned int aperfmperf_get_khz(int cpu);

#endif /* ARCH_X86_CPU_H */
MODULE_LICENSE("GPL");
#endif


