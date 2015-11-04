#ifndef MYPCB_H
#define MYPCB_H

#define MAX_STACK_SIZE 1024
#define TIMESLICE	1000		//1 million
enum State{TASK_STOP = -1, TASK_RUN = 0, TASK_WAIT = 1};

typedef unsigned int uint32_t;
struct Thread
{
	uint32_t *ip;
	uint32_t *sp;	
};

typedef struct tPCB
{
	unsigned int pid;
	enum State state;
	uint32_t *task_entry;
	uint32_t *stack;
	struct Thread thread;
	struct tPCB *next;
}tPCB;

extern tPCB task_idle;
extern tPCB *my_current;

#endif
