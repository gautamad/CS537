#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"
#include <time.h>
#include <sys/stat.h>


/*
While running the program, if the required details 
are not entered as command line arguments, then the 
function error_msg is called and then 
*/
void error_msg(char *prog,int err_code) 
{
   if(err_code == 0)
   fprintf(stderr, "Usage: fastsort -i inputfile -o outputfile\n");
   else
   fprintf(stderr,"Error: Cannot open file %s\n",prog);
   exit(1);
}

int compare_values(const void *p1, const void *p2)
{
return ((*(rec_t *)p1).key - (*(rec_t *)p2).key);

}

int
main(int argc, char *argv[])
{
    char *inFile = "no/such/file",*outFile = "no/such/file";
    clock_t end,begin;
    int i_count = 0,l_count=0,i,cl_args,file_size,input_fd,output_fd,rc;
    struct stat size_buf;	
    
    opterr = 0;
    while ((cl_args = getopt(argc, argv, "i:o:")) != -1) {
	switch (cl_args) {
	case 'i':
	    inFile = strdup(optarg);
	    i_count++;
	    break;
	case 'o':
	    outFile = strdup(optarg);
	    i_count++;
	    break;
	default:
	    error_msg(argv[0],0);
	}
    }
    if(i_count<2)
	{
	   error_msg(argv[0],0);
	}
    
    
    // open and create output file
    input_fd = open(inFile, O_RDONLY);
    if (input_fd < 0) {
	error_msg(inFile,1);
	exit(1);
    }
    	
    fstat(input_fd, &size_buf);
    file_size = size_buf.st_size;
    rec_t *sort_samp = (rec_t *)malloc(file_size);

    while (1) {	
	rc = read(input_fd, &sort_samp[l_count], sizeof(rec_t));
	l_count++;
	if (rc == 0) // 0 indicates EOF
	    break;
	if (rc < 0) {
	    error_msg(inFile,1);
	}
  }
	begin = clock();
        qsort(sort_samp,file_size/sizeof(rec_t),sizeof(rec_t),compare_values);
	end = clock();
	printf("%f\n",((double)(end-begin))/CLOCKS_PER_SEC);
     (void) close(input_fd);
 

output_fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
if (output_fd < 0) {
	error_msg(outFile,1);
	exit(1);
    }



for(i = 0;i<file_size/sizeof(rec_t);i++)
{

 rc = write(output_fd, &sort_samp[i], sizeof(rec_t));
if (rc != sizeof(rec_t)) {
    error_msg(outFile,1);
}
}
free(sort_samp);
(void) close(output_fd);
exit(0);

}


