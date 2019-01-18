/*
 *  linux/mykernel/myinterrupt.c
 *
 *  Kernel internal my_timer_handler
 *
 *  Copyright (C) 2013  Mengning
 *
 */
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/tty.h>
#include <linux/vmalloc.h>

#include "mypcb.h"

extern tPCB task[MAX_TASK_NUM];
extern tPCB * my_current_task;
extern volatile int my_need_sched;
volatile int time_count = 0;

/*
 * Called by timer interrupt.
 * it runs in the name of current running process,
 * so it use kernel stack of current running process
 */
void my_timer_handler(void)
{//内核定时调度
#if 1
    if(time_count%1000 == 0 && my_need_sched != 1)
    {
        printk(KERN_NOTICE ">>>my_timer_handler here<<<\n");
        my_need_sched = 1;
    } 
    time_count ++ ;  
#endif
    return;  	
}

void my_schedule(void)
{
    tPCB * next;
    tPCB * prev;

    if(my_current_task == NULL 
        || my_current_task->next == NULL)
    {
    	return;
    }
    printk(KERN_NOTICE ">>>my_schedule<<<\n");
    /* schedule *///切换到下一个任务
    next = my_current_task->next;
    prev = my_current_task;
    if(next->state == 0)/* -1 unrunnable, 0 runnable, >0 stopped */
    {        
    	my_current_task = next; 
    	printk(KERN_NOTICE ">>>switch %d to %d<<<\n",prev->pid,next->pid);  
    	/* switch to next process */
    	asm volatile(	
        	"pushl %%ebp\n\t" 	    /* save ebp */ // 把ebp 压入栈  esp +1 
        	"movl %%esp,%0\n\t" 	/* save esp */ // 把esp 存放到上一任务的 thread.sp 内存中
        	"movl %2,%%esp\n\t"     /* restore  esp *///把esp指向到下一任务的sp中,第一次 &task[i].stack[KERNEL_STACK_SIZE-1] 之后为上次设置
        	"movl $1f,%1\n\t"       /* save eip *///设置下一次执行的位置为下面的1:位置
        	"pushl %3\n\t" //把下一任务的ip压入栈,返回后会pop栈,并执行 (第一次 my_process 之后为 1:这个位置)
        	"ret\n\t" 	            /* restore  eip */
        	"1:\t"                  /* next process start here *///大于1次后执行的位置
        	"popl %%ebp\n\t"//弹出到ebp 等于原因原来的ebp 和 esp 指向,等于从新回到保存的进程中
        	: "=m" (prev->thread.sp),"=m" (prev->thread.ip)
        	: "m" (next->thread.sp),"m" (next->thread.ip)
    	); 
    }  
    return;	
}

