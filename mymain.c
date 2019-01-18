/*
 *  linux/mykernel/mymain.c
 *
 *  Kernel internal my_start_kernel
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

tPCB task[MAX_TASK_NUM];
tPCB * my_current_task = NULL;
volatile int my_need_sched = 0;

void my_process(void);


void __init my_start_kernel(void)
{
    int pid = 0;
    int i;
    /* Initialize process 0*/
    task[pid].pid = pid;
    task[pid].state = 0;/* -1 unrunnable, 0 runnable, >0 stopped */
    task[pid].task_entry = task[pid].thread.ip = (unsigned long)my_process;//函数地址 task_entry 不变 ip 调度时候用
    task[pid].thread.sp = (unsigned long)&task[pid].stack[KERNEL_STACK_SIZE-1];//栈内存起始位
    task[pid].next = &task[pid];//下一任务指向
    /*fork more process */
    for(i=1;i<MAX_TASK_NUM;i++)
    {//多个任务
        memcpy(&task[i],&task[0],sizeof(tPCB));
        task[i].pid = i;
	//*(&task[i].stack[KERNEL_STACK_SIZE-1] - 1) = (unsigned long)&task[i].stack[KERNEL_STACK_SIZE-1];
	task[i].thread.sp = (unsigned long)(&task[i].stack[KERNEL_STACK_SIZE-1]);//每个进程分配各自的栈内存
        task[i].next = task[i-1].next;
        task[i-1].next = &task[i];
    }
    /* start process 0 by task[0] */
    pid = 0;
    my_current_task = &task[pid];
	asm volatile(
    	"movl %1,%%esp\n\t" 	/* set task[pid].thread.sp to esp */ //把task的栈设置到esp 即栈顶
    	"pushl %1\n\t" 	        /* push ebp */ //把栈顶压入堆栈 edp 
    	"pushl %0\n\t" 	        /* push task[pid].thread.ip *///把 ip(把任务0的入口即my_process压入栈顶)
    	"ret\n\t" 	            /* pop task[pid].thread.ip to eip *///返回后会从 my_process 执行
    	: 
    	: "c" (task[pid].thread.ip),"d" (task[pid].thread.sp)	/* input c or d mean %ecx/%edx*/
	);//第一步
} 

int i = 0;

void my_process(void)
{    //第二步
    while(1)
    {
        i++;
        if(i%10000000 == 0)
        {
            printk(KERN_NOTICE "this is process %d -\n",my_current_task->pid);
            if(my_need_sched == 1)//my_need_sched 在 my_timer_handler 函数中 有内核调度,超过一定时间会设置为1
            {
                my_need_sched = 0;
        	    my_schedule();//进入调度程序
        	}
        	printk(KERN_NOTICE "this is process %d +\n",my_current_task->pid);
        }     
    }
}
