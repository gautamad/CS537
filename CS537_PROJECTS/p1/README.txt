FASTSORT.C:

i)   The input and output filenames are received as command line arguments.
ii)  If either of the names are not given, then the error, "Usage: {{filename}} -i inputfile -o outputfile" is displayed.
iii) The input file containing the records is opened using open() command.
iv)  The size of the input file is calculated using fstat command and corresponding memory is allocated on runtime using malloc and the starting address is stored in a pointer of type 'rec_t' i.e., It can be viewed as an array of structures with each element representing a record(type rec_t).
v)   Then each record is individually read from the file and stored in the allocated memory one by one.
vi)  The qsort is used to sort the records based on the key values of the records in ascending order.

vii) SORTING:
     1) qsort sorts elements in continous datastructure like an array.
     2) It requires Baseaddress of the array, total number of elements in the array, size of each array element and the address of the function in which the type of the sort desired can be defined by the user using two pointer variables each of which denotes consecutive elements of the array to be sorted(first argument - first element, second argument - next element) as arguments.
     3) In my program, the following are passed as respective arguments
	Baseaddress - starting address of the memory allocated during runtime i.e., starting address of the array of structures.
        Total size of the array - Total file size / Size of a record.
	Size of each element - Size of a record(size of struct rec_t).
	Address of the function - Address of the function 'compare_values'.
     4) compare_values(const void *p1, const void *p2) - 
		 'compare_values' function returns the value of (*(rec_t *)p1).key - (*(rec_t *)p2).key i.e., The difference between the key value of the first record and the next record. If the value returns a negative value, that order will be retained. Since we require ascending order, the difference statement is written in a such a way that the first record in the order of occurence is made minuend and the next element the subtrahend.

viii) Thus the qsort sorts the elements of an array without changing the order of values within an element.
ix) Now, in the allocated memory we have records sorted based on the key value.
x) Finally, the output file is opened and the records are written one by one and that file is closed.


XV6:

i) The Counter value should incremented every time a system call is being made. This is achieved by having a counter value (variable name : 'sys_counter') incremented everytime a system call is made from 'syscall()'. This variable has been declared an extern int in 'proc.h' and included in syscall.c. Further, 'sys_counter' is initialized to Zero in this file globally.
ii) The definition for the sys_getsyscallinfo() is written in sysproc.c which simply returns the 'sys_counter' value.
iii) Changes were made in the following files to accomodate the new getsyscallinfo():
     1)kernel/sysfunc.h - The prototype of the function sys_getsyscallinfo() is defined in _SYSFUNC_H_
     2)include/syscall.h - A system call number 22 is assigned to a variable 'SYS_getsyscallinfo' which is used to map to the sys_getsyscallinfo as the system call to be made from syscall() function.  
     3)user/user.h - The function name getsyscallinfo() prototype is defined in _USER_H_
     4)user/usys.S - SYSCALL(getsyscallinfo) is added.
iv) A user function 'get_syscounter.c' is written when used returns the total number of system calls made from the time system reboots.(optional - Just to check)


  

		 		 
