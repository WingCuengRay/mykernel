#include "create_task.h"
#include <linux/slab.h>

static void insert_to_tasklist(tPCB *new_task);
static int pid_unique(uint32_t pid);


/**
 *	@description	创建新的任务
 */
tPCB* create_task(uint32_t pid, uint32_t *entry)
{
	tPCB *new_task;
	if(!pid_unique(pid))
		return (tPCB *)0;

	new_task = kmalloc(sizeof(tPCB), GFP_KERNEL);
	new_task->pid = pid;
	new_task->state = TASK_WAIT;
	new_task->stack = kmalloc(sizeof(char)*MAX_STACK_SIZE,GFP_KERNEL);
	new_task->thread.ip = new_task->task_entry = (uint32_t *)entry;
	new_task->thread.sp = &new_task->stack[MAX_STACK_SIZE];
	insert_to_tasklist(new_task);

	return new_task;
}


/**
 *	@description	将新的PCB插入任务链表
 */
static void insert_to_tasklist(tPCB *new_task)
{
	new_task->next = my_current->next;
	my_current->next = new_task;
	
}

static int pid_unique(uint32_t pid)
{
	tPCB *p = my_current->next;
	while(p != my_current)
	{
		if(p->pid == pid)
			return 0;
		p = p->next;
	}

	return 1;
}
