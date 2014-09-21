#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
/*
struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;
*/
 

int m_w = 0x7845ff2a;    /* must not be zero, nor 0x464fffff */
int m_z = 0x45acbd23;    /* must not be zero, nor 0x9068ffff */


static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  p->tickets = 1;
  release(&ptable.lock);

  // Allocate kernel stack if possible.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  acquire(&ptable.lock);
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;
  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  
  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;
  np ->  high = 0;
  np -> low = 0;
  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);
 
  pid = np->pid;
  np->state = RUNNABLE;
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  iput(proc->cwd);
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}
//CHECK FOR THE VACANT POSITION IN THE QUEUE
//FOR THE PROCESS ADDRESS TO BE INSERTED

void queue_ins(struct proc **pro_queue,struct proc *to_be_ins)
{
int i;
for(i=0;i<NPROC;i++)
  {
    if(*(pro_queue+i) == (struct proc *) NULL )
	break;
    else if(pro_queue[i]->state != RUNNABLE && pro_queue[i]->state != RUNNING)
	break;
  }
pro_queue[i] = to_be_ins;
//cprintf("IN HIGH QUEUE :: \n");
//for(i=0;i<NPROC&&pro_queue[i]!=NULL;i++)
//cprintf("\nPROCESS IN HIGH QUEUE::/s",pro_queue[i]->name);
}


void pro_remove(struct proc **pro_queue,int rem_index)
{
  int i=rem_index;
  pro_queue[rem_index] = NULL;

  while(i < NPROC-1)
   {
    pro_queue[i] = pro_queue[i+1];
    i++;
    
   }
  pro_queue[i] = NULL;


}

 
int get_random()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  
}
int pro_select(struct proc **pro_queue)
{
 int i=0,total=0,j=0,k=0,que_2d[64][2];
 uint ran = get_random();
 while(i<NPROC)
 {
 if(pro_queue[i] != (struct proc *) NULL )
{ 
if(pro_queue[i]->state == RUNNABLE)
{ //total_tics[i] = total + pro_queue[i]->tickets; 
 que_2d[j][0]  = total+pro_queue[i]->tickets;
 que_2d[j][1] = i;
 total = que_2d[j][0];
j++;
}
}
 i++;
 
 }

 ran = ran % total;

 for(k = 0; k<j; k++)
 {
  if(que_2d[k][0] > ran)
   return que_2d[k][1];
 }
cprintf("returning O");
return 0;
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p,*sp;
  struct proc *HIGH_QUEUE[NPROC] ,*LOW_QUEUE[NPROC] ;
  int j,h_flag,l_flag,h_index,l_index;
  for(j =0 ;j<NPROC;j++)
   {
	HIGH_QUEUE[j] = NULL;
	LOW_QUEUE[j] = NULL;
   }
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
	{
        continue;
	//cprintf("while check :: process %s and state %d",p->name,p->state);
	}
       h_flag = 0;
       l_flag = 0;
       h_index = 0;
       l_index = 0;
       sp = p;
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      if((p->low)%2)
	{
  	 sp = p;
//	 cprintf("\nPROCESS::%s RUNNING SECOND TIME",p->name);
	l_flag =1;
	}
      else if(!p->high)
	{
	//place it in queue
	queue_ins(HIGH_QUEUE,p);	
	//select the process(return the index of high queue and stored in h_index)
	//sp = HIGH_QUEUE[h_index]
	// HIGH_QUEUE[0] = p;
	h_index = pro_select(HIGH_QUEUE);
	 sp = HIGH_QUEUE[h_index];
	 h_flag = 1;

	}
      else
	{
	 //place it in queue
	 //select the process(return the index of low queue and stored in h_index)
	 //sp = HIGH_QUEUE[l_index]
	 int tmp=0,ins_flag = 0,hi_pres_flag = 0,low_pres_flag = 0;
	 if(!p->low)
 	 {
	   while(tmp <NPROC)
	   {
	     if(HIGH_QUEUE[tmp] != (struct proc *)NULL)
		if(HIGH_QUEUE[tmp] -> pid == p->pid)
		  {
		   // break;
		    hi_pres_flag =1;
		    break;
		  }
	     tmp++;
	   }
	   //while(HIGH_QUEUE[tmp]->pid != p->pid && tmp != NPROC)
//		tmp++;
	  if(hi_pres_flag)
	   //HIGH_QUEUE[tmp] = NULL;
	   pro_remove(HIGH_QUEUE,tmp);
	   queue_ins(LOW_QUEUE,p);
	   low_pres_flag = 1;
	   ins_flag = 1;
	   tmp = 0;
	 }
	 if(!ins_flag)
	 {
	  while(tmp<NPROC)
	 {
	  if(LOW_QUEUE[tmp] != (struct proc *) NULL)
	   {
	     if(LOW_QUEUE[tmp]->pid == p->pid)
	     {
		low_pres_flag = 1;
	        break;	
	     }
   	   }
	  tmp++;
	 }
	 }
	if(!low_pres_flag)
	  queue_ins(LOW_QUEUE,p);
	 l_index = pro_select(LOW_QUEUE);
	 sp = LOW_QUEUE[l_index];
	 l_flag = 1;
	
	}
      if(sp->state != RUNNABLE)
	{
	 cprintf("STATE :: %s",sp->name);
	 continue;
	}
	
      proc = sp;
      switchuvm(sp);
      sp->state = RUNNING;
      swtch(&cpu->scheduler, proc->context);
      switchkvm();
      // Process is done running for now.
      // It should have changed its p->state before coming back.
      if(h_flag)
	sp->high++;
      else
	sp->low++;
      if(sp->low%2 && sp->state != ZOMBIE && sp->state!=SLEEPING)
	p=sp-1;
      proc = 0;
      if(sp->state == ZOMBIE || sp->state == SLEEPING)
	{
	 if(h_flag)
	 {
	  //HIGH_QUEUE[h_index] = NULL;
	  pro_remove(HIGH_QUEUE,h_index);
	  //HIGH_QUEUE[h_index] =NULL;
	 }
	 else
	{	 
	// LOW_QUEUE[l_index] = NULL;
	  pro_remove(LOW_QUEUE,l_index);
	  //LOW_QUEUE[l_index] =NULL;
	}
	}
	// cprintf("PROCESS :: %s HIGH :: %d LOW :: %d",p->name,p->high,p->low);
/*	if(p+1 == &ptable.proc[NPROC])
	{
	  cprintf("PROCESS ");
	}
*/	
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);
  
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}


