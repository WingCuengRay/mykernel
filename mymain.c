/*
 *  linux/mykernel/mymain.c
 *
 *  Kernel internal my_start_kernel
 *
 *  Copyright (C) 2013  Mengning
 *
 */
#include <linux/types.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/stackprotector.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/initrd.h>
#include <linux/bootmem.h>
#include <linux/acpi.h>
#include <linux/tty.h>
#include <linux/percpu.h>
#include <linux/kmod.h>
#include <linux/vmalloc.h>
#include <linux/kernel_stat.h>
#include <linux/start_kernel.h>
#include <linux/security.h>
#include <linux/smp.h>
#include <linux/profile.h>
#include <linux/rcupdate.h>
#include <linux/moduleparam.h>
#include <linux/kallsyms.h>
#include <linux/writeback.h>
#include <linux/cpu.h>
#include <linux/cpuset.h>
#include <linux/cgroup.h>
#include <linux/efi.h>
#include <linux/tick.h>
#include <linux/interrupt.h>
#include <linux/taskstats_kern.h>
#include <linux/delayacct.h>
#include <linux/unistd.h>
#include <linux/rmap.h>
#include <linux/mempolicy.h>
#include <linux/key.h>
#include <linux/buffer_head.h>
#include <linux/page_cgroup.h>
#include <linux/debug_locks.h>
#include <linux/debugobjects.h>
#include <linux/lockdep.h>
#include <linux/kmemleak.h>
#include <linux/pid_namespace.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/idr.h>
#include <linux/kgdb.h>
#include <linux/ftrace.h>
#include <linux/async.h>
#include <linux/kmemcheck.h>
#include <linux/sfi.h>
#include <linux/shmem_fs.h>
#include <linux/slab.h>
#include <linux/perf_event.h>
#include <linux/file.h>
#include <linux/ptrace.h>
#include <linux/blkdev.h>
#include <linux/elevator.h>

#include <asm/io.h>
#include <asm/bugs.h>
#include <asm/setup.h>
#include <asm/sections.h>
#include <asm/cacheflush.h>

#ifdef CONFIG_X86_LOCAL_APIC
#include <asm/smp.h>
#endif

#include "myPCB.h"
#include "create_task.h"

struct tPCB *my_current;
struct tPCB task_idle;

void idle(void);
void my_schdule();


void task1()
{
	uint32_t cnt;
	while(1)
	{
		printk(KERN_NOTICE "This is Task 111111111111 !\n");
		my_schdule();
	}
}

void task2()
{
	while(1)
	{
		printk(KERN_NOTICE "This is Task 222222222222 !\n");
		my_schdule();
	}
}

void __init my_start_kernel(void)
{
	task_idle.pid = 0;
	task_idle.state = TASK_WAIT;
	task_idle.stack = kmalloc(sizeof(char)*MAX_STACK_SIZE,GFP_KERNEL);
	task_idle.thread.ip = task_idle.task_entry = (uint32_t *)idle;
	task_idle.thread.sp = &task_idle.stack[MAX_STACK_SIZE];
	task_idle.next = &task_idle;

	task_idle.state = TASK_RUN;
	my_current = &task_idle;

	if(!create_task(1, task1))
		printk(KERN_ALERT "Create task error");
	if(!create_task(2, task2))
		printk(KERN_ALERT "Create task error");

	__asm__ __volatile__(
		"movl %0, %%esp\n\t"
		"push %0\n\t"		//这次压栈其实是为了保存栈底的指针。因为任务未运行时是空栈，所以栈顶与栈底指向同一个地址.
							//当该任务再次被调用时，通过thread.sp取得栈顶，便可通过出栈取得栈底指针（虽然此函数理论上不会再被执行）
		"pushl %1\n\t"
		"ret\n\t"
		"popl %%ebp"		//对应上面的 push %0
		:
		:"r"(task_idle.thread.sp), "r"(task_idle.thread.ip)
	);

}


void idle(void)
{
	uint32_t cnt = 0;
	while(1)
	{
		if(cnt%1000000 == 0)
		{
			printk(KERN_NOTICE "This is the idle task!Count: %d\n", cnt);
			my_schdule();
		}
		cnt++;
	}
}


void my_schdule()
{
	tPCB *next_task, *old_task = my_current;

	next_task = my_current->next;
	while(next_task != my_current)
	{
		if(next_task->state == TASK_WAIT)
			break;
		next_task = next_task->next;
	}
	if(next_task == my_current)
		return;

	old_task->state = TASK_WAIT;		//更新current变量
	my_current = next_task;
	my_current->state = TASK_RUN;
	__asm__ __volatile__(
		"movl $1f, %0\n\t"		//将标号1的位置保存到旧任务的ip中。当此任务再次被调度时便从标号1处开始执行
		"movl %%esp, %1\n\t"

		"movl %2, %%ebp\n\t"
		"movl %3, %%esp\n\t"
		"pushl %4\n\t"
		"ret\n\t"
		"1:"	
		:"=m"(old_task->thread.ip), "=m"(old_task->thread.sp)
		:"r"(my_current->stack[MAX_STACK_SIZE]), "r"(my_current->thread.sp), "r"(my_current->thread.ip)
	);
	
	return;
}
