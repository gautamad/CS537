#include<stdio.h>
#include<stdlib.h>
#include "mem.h"
#include<unistd.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

typedef struct free_list 
{
  int size;
  int free;
  struct free_list *next;
}free_list;

free_list *head = NULL;
int count = 0;
int m_error;

/*------------------------------------Mem_Init---------------------------------*/

int Mem_Init(int sizeOfRegion)
{
  int size, roundoff, memreq;
  count++;
  //Error Condition Check
  if(count == 2 || sizeOfRegion <= 0)
    {
      m_error = E_BAD_ARGS;
      return -1;
    }
  //Rounding of to the nearest page size
  size = getpagesize();
  roundoff = sizeOfRegion % size;
  if(roundoff == 0)
    memreq = sizeOfRegion;
  else
    memreq = (size-roundoff) + sizeOfRegion;
  
  //Call to mmap to request memory
  void *ptr = mmap(NULL,memreq, PROT_READ|PROT_WRITE,MAP_ANON|MAP_PRIVATE,-1,0);
  
  
  if(ptr == MAP_FAILED)
    {
      printf("mmap failed!!");
    }
  //Assign Head Pointer
  head = (free_list*) ptr;
  head -> size = memreq;
  head -> free = 1;
  
  return 0; 
}


/*------------------------------Mem_Alloc-----------------------------------*/

void *Mem_Alloc(int size1, int style)
{
  int total_size,sizealloc,count = 0, count0 = 0, countbestfit = 0, countworstfit = 0, countfirstfit = 0, size = 0, size2 = 0,firstcount= 0;
  if(size1 % 8 != 0)
    {
      size2 = size1 % 8;
      size = (8-size2) + size1;
    }
  else
    size = size1;
  
  total_size = size + sizeof(free_list);
  free_list *temp, *temp1, *temp2;
  temp1 = head;
  temp2 = head;
  switch(style) 
    {
      
      /*-----------------------------------FIRSTFIT---------------------------------*/
 case 2:
   while(temp1 != NULL)
     {
       if(temp1->free == 1 && temp1->size >= total_size)
	 {
	   if(temp1->size > total_size)
	     { 
	       countfirstfit++;
	       temp = temp1;
	       sizealloc = temp1 -> size;
 	       temp1 = (void*)temp1 + total_size;
	       temp1 ->next = temp -> next;
	       temp->next =temp1;
	       temp -> size = size;
	       temp -> free = 0;
	       temp = temp + 1;
	       temp1 -> size = sizealloc - total_size;
	       temp1 -> free = 1;
	  break;
	     }
	   else
	     {
	       countfirstfit++;
	       temp = temp1;
	       temp -> size = size;
	       temp -> free = 0;
	       temp = temp + 1;
	       break;
	     }
	 }
       temp1 = temp1->next;
     }
   
   if(countfirstfit == 0)
     {
       m_error = E_NO_SPACE;
       return 0;
     }
   
   break;
   
   /*--------------------------------------BESTFIT------------------------------*/
    case 0:
      while(temp2!= NULL)
	{
	  if(temp2->size == total_size && temp2->free ==1) 
	    
	    {  
	      countbestfit++;
	      count = temp2->size;
	      temp = temp2;
	      count0++;
	      temp -> size = size;
	      temp -> free = 0;
	      temp = (void*)temp + 16 ;
	      goto L1 ;
	    }
	  
	  if(temp2->size >total_size && temp2 -> free == 1)
	    {
	      if(firstcount == 0)
		{
		  firstcount++;
		  countbestfit++;
		  count = temp2 -> size;
		  temp1 = temp2;
		}
	      else if(temp2->size < count)
		{
		  count = temp2 -> size;	
		  temp1 = temp2;
		}
	    }	
	  
	  temp2 = temp2->next;
	} 
      

      
      temp = temp1;
      sizealloc = temp1 -> size;
      if((sizealloc -total_size) > 16)
	{
	  temp1 = (void*)temp1 + total_size;
	  temp1 ->next = temp -> next;
	  temp->next =temp1;
	  temp -> size = size;
	  temp -> free = 0;
	  temp = (void*)temp + 16;
	  temp1 -> size = sizealloc - total_size;
	  temp1 -> free = 1;
	}
      else
	{
	  temp -> size = size + (sizealloc -total_size);
	  temp -> free = 0;
	  temp = (void*)temp + 16;
	}
      

    L1: if(countbestfit == 0)
     {
       m_error = E_NO_SPACE;
       return 0;
     }
      
      break;
      
      
      /*--------------------------------WORSTFIT----------------------------------*/

    case 1:
      while(temp2 != NULL)
	{
	  if(temp2 -> size > total_size && count0 == 0 && temp2 -> free == 1)
	    {
	      countworstfit++;
	      count = temp2 -> size;
	      temp1 = temp2;
	      count0++;
	    }
	  if(temp2->size > count && temp2 -> size >= total_size && temp2 -> free == 1)
	    {
	      count = temp2 -> size;
	      temp1 = temp2;
	    }
	  temp2 = temp2->next;
     }
      temp = temp1;
      sizealloc = temp1 -> size;
      temp1 = (void*)temp1 + total_size;
      temp1 ->next = temp -> next;
      temp->next =temp1;
      temp -> size = size;
      temp -> free = 0;
      temp = temp + 1;
      temp1 -> size = sizealloc - total_size;
      temp1 -> free = 1;
      
      if(countworstfit == 0)
	{
	  m_error = E_NO_SPACE;
	  return 0;
	}
      
      break;
      
 }
  temp2 = head;
  if(head == NULL) return NULL;
  
  
  
  return (temp);
}

/*-------------------------------FREE--------------------------------*/
int Mem_Free(void *ptr)
{
  free_list *temp1,*temp2,*temp3,*temp4,*temp5,*temp6,*final,*badfree, *search, *freepointer,*searchfree;
  int countbadfree = 0;
  int badfreecount = 0;
  if( ptr == NULL) return -1;
  searchfree = head; 

  badfree = head;
  
  freepointer = ptr;
  freepointer = (void*)freepointer - 16;
  if(head == freepointer)
    {
      head -> free = 1;
      head -> size+=sizeof(free_list);
      return 0;
    }
  while(searchfree)
    {
      if( head == freepointer)
	{
	  head -> free = 1;
	  head -> size += sizeof(free_list);
	}
      if(searchfree -> next == freepointer)
 	{	
	  badfreecount++;
	  if(searchfree -> free == 1 && searchfree -> next ->next->free ==1)
	    {	
	      searchfree->size = searchfree ->size +sizeof(free_list)+freepointer->size + searchfree ->next->next->size;
	      searchfree -> next = searchfree ->next->next->next;
	      searchfree -> free = 1;
	      break;
	    }
	  else if(searchfree -> free == 1)
	    {
	      searchfree->size = searchfree->size  + searchfree->next->size+ sizeof(free_list);
	      searchfree->next = searchfree->next->next;
	      searchfree -> free = 1;
	      break;
	    }
	  else if(searchfree -> next -> next ->free == 1)
	    {
	      freepointer -> size = searchfree->next->size + searchfree->next->next->size + sizeof(free_list);
	      freepointer -> next = freepointer ->next->next;
	      freepointer -> free = 1;
	      break;
	    }
	  else
	    {
	      freepointer -> free = 1;
	      freepointer -> size = freepointer ->size + sizeof(free_list);
	      break;
	    }
	}
      searchfree = searchfree ->next;	
    }
  
  if(badfreecount==0)
    {
      m_error =E_BAD_POINTER;
      return -1;
    }
  
  
      return 0;
}


/*-----------------------------------DUMP---------------------------------------*/

void Dump()
{
  //printf("Inside Dump \n");
  free_list *temp = head;
  while(temp)
    {
      
     printf(" The spaces are %d starting address %p  if free: %d \n",temp -> size, temp, temp -> free);
     //printf(" The address is %p \n", temp);
     temp = temp -> next;
    }
}

