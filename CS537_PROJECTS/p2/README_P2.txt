
----AUTHOURSHIP-----

Dineshbabu Selavaraju (dineshba@cs.wisc.edu)
Gautam Dhanabalan (dhanabal@cs.wisc.edu)

Description:

SHELL:

i) INTERACTIVE MODE : Command is received from the user using fgets.
   BATCH MODE : Commands are read from the file and stored in a 2d array and processed one by one by going through the code in a loop till the EOF is reached or exit command is met with higher priority for the latter case.
ii)and_count (Number of &s in a single command line seperating various commands) is taken to execute multiple commands in a line seperated by '&'.
iii)arg_count contains the number of arguments in a command. eg., ls -lrth ==> arg_count = 2.
iv) & check:
	Checks for the presence of & flag and run the process in background by setting fin_and_flag to '1' which is read by parent process to decide whether to wait for the process or proceed with its functionality(Get ready to execute the next command).
v) EXIT check:
	If the received command is 'exit' the shell is terminated by using exit(0) command.
vi) CD check:
	If the cd command has arguments then the received argument is the path to be set as Current Directory and this is done using chdir.
	If there is no argument for the cd command, then the home directory(got by 'getenv') is set as Current Directory.
vii) PWD check:
	If no argument was received along with pwd command then the Present Working Directory is printed using getcwd()
	If there is any argument with pwd, then error message is thrown.
viii)PYTHON check:
	If any python file is received(ending with .py) then the file name is appended to the character 'python' and proceed with execvp.
ix) Wait Check:
	If wait command is received, then the code is written in a such a way that the parent waits till all the child processes are terminated. wait command waits till a child process gets terminated and it is executed in a loop. The loop is broken when the number of such terminated process reaches the ECHILD count and it is checked from errno.
x)Redirection check:
	The command containing > symbol is subjected to redirection check. Various error cases are checked and the command and the destination file is split. The output stream is set to the destination file and the command is executed using EXECVP.
	For all the commands except exit, cd, pwd and wait, CHILD process is forked and executed using execvp command.

XV6:

xv6 Scheduler Project was implemented in 2 stages

1st Stage: 
The system calls 'settickets' and 'getpinfo' were declared in appropriate files (syscall.h, sysfunc.h, syscall.c , sysfile.c, usys.S, user.h)
The system call settickets is defined in sysproc.c to set the numbers of tickets for each process and return 0 otherwise return -1 if the tickets weren't set for a process.
The system call getpinfo is defined in sysproc.c to get information about all process currently present in the process table.


2nd Stage:
- The scheduler algorithm was implemented in the 2nd stage of the project.
- Our scheduler algorithm firstly populates the RUNNABLE processes in the process table into high and low priority queues based on the hticks and lticks.
- For a new process arriving it is put inside a high priority queue and run for 1 time slice. Once a process is run on high priority for a time slice it is then moved from       high priority to low priority and updated on the low priority queue.
- If a process has run odd number of times on low priority is run again for a time slice.
- When there are multiple items in either the high priority queue or the low priority queue a lottery scheduler is used to decide the process that gets to run.
- To implement the lottery a scheduler the tickets from each runnable process is summed up and added to a 2-Dimensional Array and then the random generator generates a random    number to pick the winner among the runnable processes in either the high or low priority queue.
- When once a process goes to SLEEPING or ZOMBIE state it is removed from either of the queue.


The lottery scheduler performed as expected giving more CPU time to processes with higher number of tickets. 


Dineshbabu Selvaraju
Gautam Dhanabalan
