#include "cs537.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// CS537: Parse the new arguments too
char *global_schedalg;
void getargs(int *port, int *thread_no,int *buf_len,char *schedalg,int argc, char *argv[])
{
    if (argc != 5) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *thread_no = atoi(argv[2]);
    *buf_len = atoi(argv[3]);
     strcpy(schedalg,argv[4]);
     global_schedalg = argv[4];
}
pthread_mutex_t mut;
pthread_cond_t prod,cons;
int count = 0;
typedef struct buf
{
int info;
char cgiargs[MAXLINE],filename[MAXLINE],method[MAXLINE];
int is_static;
struct stat sbuf;
struct buf *next;
}struct_buf;
struct_buf *head = NULL;


void thread_func(int number)
{
int val;
struct_buf *req;
struct_buf * buf_get(void);
while(1)
{
pthread_mutex_lock(&mut);
while(count == 0)
{
pthread_cond_wait(&cons,&mut);
}
req =  (struct_buf *)buf_get();
pthread_cond_signal(&prod);
pthread_mutex_unlock(&mut);
requestHandle(req->info,req->filename,req->method,req->is_static,req->sbuf,req->cgiargs);
Close(req->info);
free(req);
}

}
int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen,thread_no,buf_len;
    struct sockaddr_in clientaddr;
    char *schedalg = (char *)malloc(strlen(argv[4]));
    int i;
    void buf_insert(int,char *);
    //struct buf *trav;
    getargs(&port,&thread_no,&buf_len, schedalg,argc, argv);
    Pthread_mutex_init(&mut,NULL);
    Pthread_cond_init(&prod,NULL);
    Pthread_cond_init(&cons,NULL);
    // 
    // CS537: Create some threads...
    //
    pthread_t t[thread_no];
     listenfd = Open_listenfd(port);
    for(i=0;i<thread_no;i++)
    {
     Pthread_create(&t[i],NULL,thread_func,i);
    }
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
	Pthread_mutex_lock(&mut);
        while(count >= buf_len)
{
	printf("QUEUE FULL");
	Pthread_cond_wait(&prod,&mut);
}
	
	buf_insert(connfd,schedalg);
	Pthread_cond_signal(&cons);
	Pthread_mutex_unlock(&mut);
	// 
	// CS537: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. However, for SFF, you may have to do a little work
	// here (e.g., a stat() on the filename) ...
	// 

}

}
void buf_insert(int info,char *schedalg)
{

struct_buf *new = (struct_buf *)malloc(sizeof(struct_buf)),*inter;
char buffer[MAXLINE],uri[MAXLINE],version[MAXLINE];
rio_t rio;
Rio_readinitb(&rio, info);
Rio_readlineb(&rio, buffer, MAXLINE);
sscanf(buffer, "%s %s %s", new->method, uri, version);
//printf("%s %s %s\n", new->method, uri, version);
 
 requestReadhdrs(&rio);
new->is_static = requestParseURI(uri, new->filename, new->cgiargs);
stat(new->filename,&new->sbuf);

new->next = NULL;
new->info = info;
if(head == NULL)
{
head = new;
}
else
{

if(!strcmp(schedalg,"FIFO"))
{

inter = head;
while(inter->next!=NULL)
inter = inter->next;
inter->next = new;

}


else if(!strcmp(schedalg,"SFNF"))
{
inter = head;
while(inter->next!=NULL)
inter = inter->next;
inter->next = new;
}

else
{
inter = head;
while(inter->next!=NULL)
inter = inter->next;
inter->next = new;


}
}
count++;
}


struct_buf *buf_get()
{
int val;
char *inter_fname;
int fsize,fname_size;
struct_buf *req,*del;
if(head == NULL)
	{
	fprintf(stdout,"NOT WORKING");
	exit(0);
	}
else
	{
	if(!strcmp(global_schedalg,"FIFO"))
		{
		del = head;
		val = head->info;
		head=head->next;
		count--;
		return del;
	}
else if(!strcmp(global_schedalg,"SFNF"))
	{
		if(count == 1)
{
del = head;
head = NULL;
count--;
return del;
}
else
{
        req = head;
        del = head;
        fname_size = strlen(req->filename);
        do
                {
                if(fname_size>strlen(req->filename))
                        {
                        fname_size = req->filename;
                        del = req;
                        }
                req = req->next;
                } 
        while(req !=NULL);
        req = head; 
        while(req->next != del && del != head)
                {
                req = req->next;
                }
        if(del == head) 
         head = head->next;
        else
        req->next = del->next;
        count--;
        return del;
}

	  
	}
else
	{

if(count == 1)
{
del = head;
head = NULL;
count--;
return del;
}
else
{
	req = head;
	del = head;
	fsize = req->sbuf.st_size;
	do
		{
		if(fsize>req->sbuf.st_size)
			{
			fsize = req->sbuf.st_size;
			del = req;
			}
		req = req->next;
		}
	while(req !=NULL);
	req = head;
	while(req->next != del && del != head)
		{
		req = req->next;
		}
	if(del == head)
	 head = head->next;
	else
	req->next = del->next;
	count--;
	return del;
}
}


}
} 
