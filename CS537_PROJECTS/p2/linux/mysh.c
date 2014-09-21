#include<stdio.h>
#include<stdlib.h>
   #include <unistd.h>
#include <ctype.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>

int split2(char *val, char **argc)
{
int count = 0;
while(1)
{
while(*val == ' ' || *val == '\t')
{
*val='\0';
val++;
}
if(*val == '\0')
{
argc[count] = '\0';
return count;
}
argc[count] = val;
count++;

while(*val!=' ' && *val!= '\0' )
val++;

}


}

int redir_check(char *val)
{
 int count = 0;
 while(*val!='\0')
   {
		
	if(*val=='>')
            count++;
	val++;
	

   }
return count;

}
int and_count_fn(const char *val)
{
   int count = 0;
   const char *and_pos;
   while(*val != '\0')
	{
	 if(*val == '&')
	{
	   count++;
	   and_pos = val;
	}
	 
	 val++;
	}
	if(val - 1 == and_pos)
	 return count;
	else if(count)
	 return count+1;
	else
	 return 0;

}


int split_and(char *val, char **arg)
{
  int count=0,fin_pre = 0;
  arg[count] = val;
  while(*val != '\0')
	{
          fin_pre = 0;
	  while(*val != '&'&&*val != '\0')
	  {
  		val++;
		fin_pre = 1;
	  }
	  if(*val == '\0')
	   { 
	     
	     return 0;
	   }
	
	  *val = '\0';
	  count++; 
	  val++;
	  arg[count] = val; 


	}
  



return fin_pre;
}




int main(int argc, char* argv[])
{
int bt_flag = 0,batch_count = 0,line_num = 0,char_num = 0, and_loop_count = 0;
char arg_list[512][1000],*path,c;
char error_message[30] = "An error has occurred\n";


FILE *fp;
if(argc == 2)
	{	
	if(argv[1] != '\0')// && argv[1][0] == '[' && argv[1][strlen(argv[1])-1] == ']')
		{
			//path = argv[1]+1;
			//argv[1][strlen(argv[1])-1] = '\0';
			path = argv[1];
			fp = fopen(path,"r");
			if(!fp)
			{
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(1);
			}
			bt_flag = 1;
			while((c=fgetc(fp)) != EOF )
			{		 
			  if(c == '\n')
			   {    
	  			arg_list[line_num][char_num] = '\0';
			//	arg_list[line_num][512] = '\0';		
				char_num = 0;			
				line_num++;
				continue;
			   }
			   arg_list[line_num][char_num] = c;
			   char_num++;
			}


		}
	else
		{
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
		}
	}
else if(argc > 2)
	{
	
	write(STDERR_FILENO, error_message, strlen(error_message));
	exit(1);
	}

while(1)
{

char hell[513],*cmd_wrd[512], *path,*and_arr[512],and_list[512],wait_flag = 0,cwd_path[100];
static int multi_and_flag,and_sym_count,fin_pre; 
/*and_sym_count is the number of commands in a single line seperated by &
  fin_pre tells whether the final command ends with &
  and_count is the number of commands seperated by &  in a line
  multi_and_flag tells whether there are multiple commands in a single line seperated by &
  and_arr is an array of character pointers with each element pointing to the command seperated by &
  and_list is the string with multiple commands in a single line
*/
int and_count = 0,fin_and_flag = 0,arg_count = 0,redir_flag = 0;
//if(!bt_flag && !fin_pre && !multi_and_flag)
if(argc<2 && !multi_and_flag)
write(1,"mysh> ",6);
if(!bt_flag && !multi_and_flag)
{
//	gets(hell);
fgets(hell,512,stdin);
if(feof(stdin))
 exit(0);
hell[strlen(hell)-1]='\0';
hell[512] = '\0';
}
else if(multi_and_flag)
{

	if(and_loop_count < and_sym_count)
		{
		strcpy(hell,and_arr[and_loop_count]);
		if(and_loop_count == and_sym_count-1 && !fin_pre)
			fin_and_flag = 0;
		else
			fin_and_flag = 1;
		and_loop_count++;
		}
	else
		{	
		//  printf("%d",fin_pre);
		  and_sym_count = 0;
		  multi_and_flag = 0;
		  and_loop_count = 0;
		  fin_pre = 0;
		  continue;
		}
}
else{

	if(batch_count == line_num)
	exit(0);
	strcpy(hell,arg_list[batch_count]);
	write(1,hell,strlen(hell));
	if(strlen(hell))
	write(1,"\n",1);
	batch_count++;
	if(strlen(hell)>512)
{
         write(STDERR_FILENO, error_message, strlen(error_message));
	 continue;
}		
//	hell[512] = '\0';
}
if(hell[0] == '\0')
	continue;
and_count = and_count_fn(hell);
redir_flag = redir_check(hell);

if(and_count <= 1)
{
arg_count = split2(hell,cmd_wrd);
}
else
{
arg_count =1;
}
if(!arg_count)
continue;

////////////////////////////////////////////////////////////& CHECK //////////////////////////////
if(and_count)
{	
	strcpy(and_list,hell);
	fin_pre = split_and(and_list,and_arr);
	if(and_count == 1)
	{
		
		if(!strcmp(cmd_wrd[arg_count-1],"&"))
		{
		  fin_and_flag = 1;
		  cmd_wrd[arg_count - 1] = '\0';
		}
		else if(cmd_wrd[arg_count-1][(int) strlen(cmd_wrd[arg_count-1])-1] == '&')
		{
		  fin_and_flag = 1;
		  cmd_wrd[arg_count-1][strlen(cmd_wrd[arg_count-1])-1] = '\0';	 	
		}
	}
	else
	{
		
		and_sym_count = and_count;
		multi_and_flag = 1;
		continue;

	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////
else if(!strcmp(cmd_wrd[0],"exit") || cmd_wrd[0] == '\0')
	if(cmd_wrd[1] =='\0')
	{	
	exit(0);
	}
	else
	{
	write(STDERR_FILENO, error_message, strlen(error_message));
	continue;
	}
	
/////////////////////////////////////////CD/////////////////////////////////////
else if(!strcmp(cmd_wrd[0],"cd"))
{
	if(cmd_wrd[1] == '\0')
	{
	 path =  getenv("HOME");

	}
	else
	{
	path = cmd_wrd[1];

	}
	if(chdir(path)<0)
	write(STDERR_FILENO, error_message, strlen(error_message));
	continue;
}
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////PWD/////////////////////////////////////
else if(!strcmp(cmd_wrd[0],"pwd"))
{
	if(cmd_wrd[1] == '\0')
	{

	 getcwd(cwd_path,sizeof(cwd_path));
	 write(STDOUT_FILENO,cwd_path,strlen(cwd_path));
	 write(STDOUT_FILENO,"\n",1);	
	}
	else
	{
		write(STDERR_FILENO, error_message, strlen(error_message));
		continue;
	}

	//if(chdir(path)<0)
	//write(STDERR_FILENO, error_message, strlen(error_message));
	continue;
}
////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////********* PYTHON **********////////////////////
else if(!strcmp(cmd_wrd[0]+strlen(cmd_wrd[0])-3,".py"))
{
int loo;
for(loo=arg_count+1;loo>0;loo--)
{
cmd_wrd[loo] = cmd_wrd[loo-1];

}
cmd_wrd[0] = "python";
}
////////////////////////////////////////////////////////////////////////////
///////////////////////////////////********wait*********/////////////////////////
else if(!strcmp(cmd_wrd[0],"wait"))
{
if(cmd_wrd[1] !='\0')
	{	
	write(STDERR_FILENO, error_message, strlen(error_message));
	continue;
	}
else
	{
	fin_and_flag = 1;
	wait_flag = 1;
	}
}
////////////////////////////////////////////////////////////////////////////


int rc = 0;
if(!wait_flag)
rc=fork();
else
rc = 1;
if(rc < 0)
	write(STDERR_FILENO, error_message, strlen(error_message));
else if(rc == 0)
{
//printf("\nchild process :: %d",getpid());







/////////////////////////////////////////////////////////////redirection///////////////////////
	if(redir_flag == 1)
	{
			 	
		 int redir_loop = 0,redir_null_count = 0;
		 if(fin_and_flag)
		   {
			if(cmd_wrd[arg_count-1] == '\0')
			 arg_count--;
		    }
		 if(arg_count!=1 && !strcmp(cmd_wrd[arg_count-2],">"))
			{
			 path = cmd_wrd[arg_count-1];
			 redir_null_count = arg_count -2;
			  
			}  
		 else if(arg_count == 1)
			{
			while(cmd_wrd[0][redir_loop]!='>')
			{  redir_loop++;}
			path = cmd_wrd[0]+redir_loop+1;
			cmd_wrd[0][redir_loop] = '\0';  
			redir_null_count = 1;
			}     
		else
		{	
			while(cmd_wrd[redir_loop] != '\0')
			{
		 
			  if(redir_check(cmd_wrd[redir_loop]))
			  {
		  		if(redir_loop < arg_count-2)
				    {
					write(STDERR_FILENO, error_message, strlen(error_message));
					exit(1);
				    }

				else if(redir_loop == arg_count-1)
				    {
					if(!strcmp(cmd_wrd[redir_loop],">"))
					  {
						write(STDERR_FILENO, error_message, strlen(error_message));
						exit(1);
					  }
					else
					  {     
						int redir_pos = 0;
						while(cmd_wrd[redir_loop][redir_pos] != '>')
							redir_pos++;
						cmd_wrd[redir_loop][redir_pos] = '\0';
						//cmd_wrd[redir_loop] = (cmd_wrd[redir_loop])+redir_pos;
						path = cmd_wrd[redir_loop]+redir_pos+1;
						redir_pos = redir_pos?1:0;
						redir_null_count = redir_loop+redir_pos;
					   }
				    }
				else
				{
		  			cmd_wrd[redir_loop][(int)strlen(cmd_wrd[redir_loop])-1] = '\0';
					path = cmd_wrd[redir_loop+1];
					redir_null_count = redir_loop+1;
				}
				   
			   break;
			
		 		           
			   }    
			 redir_loop++;	
			  }  
		
	
		}
		int close_rc = close(STDOUT_FILENO);
			 if(close_rc < 0)
			    {
	        	       write(STDERR_FILENO, error_message, strlen(error_message));
			       exit(1);
			    }
		int fd = open(path,O_RDWR | O_CREAT | O_TRUNC,S_IRWXU);
		if(fd<0)
	 	{
                 write(STDERR_FILENO, error_message, strlen(error_message));
		 exit(1);	
		}

		cmd_wrd[redir_null_count] = '\0';

	}
	else if (redir_flag > 1)
	{
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}

/////////////////////////////////////////////////////////////////////////////////

execvp(cmd_wrd[0],cmd_wrd);
write(STDERR_FILENO, error_message, strlen(error_message));
exit(1);
}
else
{

int wc=0;

if(!fin_and_flag)
	wc = waitpid(rc,NULL,__WALL);
else if(wait_flag)
	while(wait(NULL))
	{
	 if(errno == ECHILD)
		break;
	}
//	wc = waitpid(-1,NULL,__WALL);
//printf("\nParent process :: %d\n",getpid());
fin_and_flag = 0;
}




}
}
