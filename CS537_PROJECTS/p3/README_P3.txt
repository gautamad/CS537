
----AUTHOURSHIP-----

Dineshbabu Selavaraju (dineshba@cs.wisc.edu)
Gautam Dhanabalan (dhanabal@cs.wisc.edu)

Description:

Memory Allocation:

Mem_Init:

- Mem_Init returns the number bytes requested aligned to the page size for by the user in the first time. If called multiple times Mem_init return an error.
- Header for list consists parameters size, next pointer and free variable.

Mem_Alloc:
- In Mem_Alloc three different types of allocations are implemented(BESTFIRST, FIRSTFIT, WORSTFIT).
- Basically the Head pointer points to the first memory reutrned by mmap routine. 
- When a Mem_alloc call is made the list is searched and a block which is free is selected based on the three types of fits and the free variable is set to 0 for the allocated block. 
- The list has all the allocated blocks and free blocks.

Mem_Free
- In Mem_Free pointer to be freed is passed and it checks for the header. If header is not valid then bad pointer is asserted.
- If header is valid then the space is freed and the free variable in the header is set to 1.
- If previous block or the next block to the free pointer is free then the blocks are coalasced and the size of the bigger is updated taking into account the header size.

Dump
- Dump prints the items in the list. This function is used for debugging.

XV6:
Null pointer Dereference:
makefile.mk-USER_LDFLAGS value at the user/makefile.mk is set to
0x1000(Starting address of the second page).
vm.c-While creating the child process from parent process, at copyuvm where
the copy of the parent process' contents are copied, first page of the parent
process is left uncopied.
exec.c-While allocating memory for the process at allocuvm, memory is
allocated and mapped into the page table from the second page. This was done
by setting the starting address(argument to allocuvm) as PGSIZE.

Stack setup at the bottom of the VA:
exec.c - The bottom page of the VA(last page of the userspace for the
process/page above USERTOP) is allocated for stack by modifying the starting
address as the USERTOP-PGSIZE to allocuvm.
proc.h - An additional variable 'sz2' has been added in the proc structure to
keep track of the stack bounds.
trap.c - A check has been added at the point where PAGE FAULT occurs. It
checks whether the PAGE FAULT occured at the page above the current stack. If
so, a page above the current stack is allocated and the variable 'sz2' is
changed to indicate that page(sz2 value is decreased since stack is made to
grow  toward the heap)
syscall.c - Checks have been added at fetchint, fetchstr, argint, argptr,
argstr to check for bounds so that any attempt to access the FIRST PAGE, space
between HEAP and STACK throws PAGE FAULT and return valid value if it attempts
to access the rearranged AS(stack).
proc.c - A check has been added so that any attempt to grow heap beyond stack
throws error. There is a guard page between stack and heap.





Dineshbabu Selvaraju
Gautam Dhanabalan
