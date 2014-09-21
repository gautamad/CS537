#include "mfs.h"
#include "udp.h"
#include <string.h>

struct sockaddr_in saddr;
int sd;

int timeout(int t_s)
{
volatile int rd;
fd_set r;
FD_ZERO(&r);
FD_SET(t_s,&r);
struct timeval t;
t.tv_sec = 1;
t.tv_usec = 0;
rd =  select(t_s+1, &r,NULL,NULL,&t);
return rd;
}

int MFS_Init(char *hostname, int port)
{
 sd = UDP_Open(0);
 assert(sd > -1);
//while(timeout()<= 0);
 int rc = UDP_FillSockAddr(&saddr,hostname, port);
 assert(rc == 0);
return 0;
}

int MFS_Lookup(int pinum, char *name)
{

 MFS_Client_Msg clientmsg;
  MFS_Client_Msg servermsg;
 clientmsg.pinum = pinum;
 clientmsg.inum = -1;
 clientmsg.block_offset = -1;
 clientmsg.msg_type = LOOKUP;
 clientmsg.port = -1;
 clientmsg.file_type = -1;
 strcpy(clientmsg.name,name); 
 strcpy(clientmsg.buffer,"-1\0");
int rc = 0;//,t_s =sd;

while(1)
{

fd_set r;
FD_ZERO(&r);
FD_SET(sd,&r);
struct timeval t;
t.tv_sec = 5;
t.tv_usec = 0;

rc = UDP_Write(sd, &saddr, (char *) &clientmsg , sizeof(clientmsg));

rc =  select(sd+1, &r,NULL,NULL,&t);
if(rc > 0){

break;
}
}

 if (rc > 0) {
 struct sockaddr_in raddr;
//fd_set r;
//FD_ZERO(&r);
//FD_SET(sd,&r);
//struct timeval t;
//t.tv_sec = 1;
//t.tv_usec = 0;

 rc = UDP_Read(sd, &raddr,(char*) &servermsg,sizeof(servermsg));
 }
 return servermsg.ret_val;
}

int MFS_Stat(int inum, MFS_Stat_t *m)
{
 MFS_Client_Msg clientmsg;
  MFS_Client_Msg servermsg;
 clientmsg.pinum = -1;
 clientmsg.inum = inum;
 clientmsg.block_offset = -1;
 clientmsg.msg_type = STAT;
 clientmsg.port = -1;
 clientmsg.file_type = -1;
 strcpy(clientmsg.name,"\0");
 strcpy(clientmsg.buffer,"-1\0");


int rc = 0;
while(1)
{
rc = UDP_Write(sd, &saddr, (char *) &clientmsg , sizeof(clientmsg));
fd_set r;
FD_ZERO(&r);
FD_SET(sd,&r);
struct timeval t;
t.tv_sec = 5;
t.tv_usec = 0;
rc=  select(sd+1, &r,NULL,NULL,&t);



if(rc > 0){

//while(timeout()<= 0);
break;
}
}
 if (rc > 0) {
  struct sockaddr_in raddr;
//while(timeout()<= 0);

   rc = UDP_Read(sd, &raddr,(char*)&servermsg,sizeof(servermsg));
 // m = &servermsg.stat;
 m->type = servermsg.stat.type;
 m->size = servermsg.stat.size;
  }
  return servermsg.ret_val;


}


int MFS_Write(int inum, char *buffer, int block)
{
MFS_Client_Msg clientmsg;
MFS_Client_Msg servermsg; 
clientmsg.pinum = -1;
 clientmsg.inum = inum;
 clientmsg.block_offset = block;
 clientmsg.msg_type = WRITE;
 clientmsg.port = -1;
 clientmsg.file_type = -1;
 strcpy(clientmsg.name,"\0");
 memcpy(clientmsg.buffer, buffer, 4096);
//while(timeout()<= 0);

int rc = 0;
while(1)
{
 rc = UDP_Write(sd, &saddr, (char *) &clientmsg , sizeof(clientmsg));
fd_set r;
FD_ZERO(&r);
FD_SET(sd,&r);
struct timeval t;
t.tv_sec = 5;
t.tv_usec = 0;
rc =  select(sd+1, &r,NULL,NULL,&t);
if(rc > 0){

break;
}
}
 if(rc > 0)
 {
  struct sockaddr_in raddr;
//while(timeout()<= 0);

  rc = UDP_Read(sd, &raddr, (char*) &servermsg, sizeof(servermsg));
 }
if(rc > 0)
return servermsg.ret_val;
else
return -1; 
}

int MFS_Read(int inum, char *buffer, int block)
{
 MFS_Client_Msg clientmsg;
 MFS_Client_Msg servermsg;
 clientmsg.pinum = -1;
 clientmsg.inum = inum;
 clientmsg.block_offset = block;
 clientmsg.msg_type = READ;
 clientmsg.port = -1;
 clientmsg.file_type = -1;
 strcpy(clientmsg.name,"\0");
 memcpy(clientmsg.buffer,buffer,4096);
//while(timeout()<= 0);
int rc = 0;
while(1)
{
 rc = UDP_Write(sd, &saddr, (char *) &clientmsg , sizeof(clientmsg));
fd_set r;
FD_ZERO(&r);
FD_SET(sd,&r);
struct timeval t;
t.tv_sec = 5;
t.tv_usec = 0;


rc =  select(sd+1, &r,NULL,NULL,&t);
if(rc > 0){

break;
}
}
  if(rc > 0)
{
 struct sockaddr_in raddr;
//while(timeout()<= 0);

 rc = UDP_Read(sd, &raddr, (char *) &servermsg, sizeof(servermsg));
}
memcpy(buffer, servermsg.buffer,4096);

return servermsg.ret_val;
}

int MFS_Creat(int pinum, int type, char *name)
{
 MFS_Client_Msg clientmsg;
MFS_Client_Msg servermsg;
 clientmsg.pinum = pinum;
 clientmsg.inum = -1;
 clientmsg.block_offset = -1;
 clientmsg.msg_type = CREAT;
 clientmsg.port = -1;
 clientmsg.file_type = type;
 strcpy(clientmsg.name,name);
 strcpy(clientmsg.buffer,"-1\0");
//while(timeout()<= 0);
int rc = 0;

while(1)
{
rc = UDP_Write(sd, &saddr, (char *) &clientmsg , sizeof(clientmsg));

fd_set r;
FD_ZERO(&r);
FD_SET(sd,&r);
struct timeval t;
t.tv_sec = 10;
t.tv_usec = 0;
rc =  select(sd+1, &r,NULL,NULL,&t);

//rd = 0;





// rc = UDP_Write(sd, &saddr, (char *) &clientmsg , sizeof(clientmsg));

if(rc>0)
break;

}
/*
fd_set k;

FD_ZERO(&k);
FD_SET(sd,&k);
t.tv_sec = 5;
t.tv_usec = 0;*/
struct sockaddr_in raddr;
//while(1)
//{
//rc =  select(sd+1, &k,NULL,NULL,&t);
if(rc > 0)
rc = UDP_Read(sd, &raddr, (char *) &servermsg, sizeof(servermsg));
//if(rc>0)
//break;

//if(rc>0)
//break;



/*  if(rc > 0)
{
 struct sockaddr_in raddr;
//while(timeout()<= 0);


 rc = UDP_Read(sd, &raddr, (char *) &servermsg, sizeof(servermsg));
}
*/
return servermsg.ret_val;
}


int MFS_Unlink(int pinum, char *name)
{
 MFS_Client_Msg clientmsg;
MFS_Client_Msg servermsg;
 clientmsg.pinum = pinum;
 clientmsg.inum = -1;
 clientmsg.block_offset = -1;
 clientmsg.msg_type = UNLINK;
 clientmsg.port = -1;
 clientmsg.file_type = -1;
 strcpy(clientmsg.name,name);
 strcpy(clientmsg.buffer,"-1\0");
//while(timeout()<= 0);


//volatile int rd = 0;
int rc;
while(1)
{
rc = UDP_Write(sd, &saddr, (char *) &clientmsg , sizeof(clientmsg));
fd_set r;
FD_ZERO(&r);
FD_SET(sd,&r);
struct timeval t;
t.tv_sec = 5;
t.tv_usec = 0;

rc =  select(sd+1, &r,NULL,NULL,&t);



if(rc>0)
{

break;
}
}
  if(rc > 0)
{
 struct sockaddr_in raddr;
//while(timeout()<= 0);


 rc = UDP_Read(sd, &raddr, (char *) &servermsg, sizeof(servermsg));
}

return servermsg.ret_val;
}

int MFS_Shutdown()
{
 MFS_Client_Msg clientmsg;
 clientmsg.pinum = -1;
 clientmsg.inum = -1;
 clientmsg.block_offset = -1;
 clientmsg.msg_type = SHUTDOWN;
 clientmsg.port = -1;
 clientmsg.file_type = -1;
 strcpy(clientmsg.name,"\0");
 strcpy(clientmsg.buffer,"-1\0");


//int rd = 0;
int rc = 0;
//int  t_s = sd;
while(0)
{
//  rc = UDP_Write(sd, &saddr, (char *) &clientmsg , sizeof(clientmsg));
fd_set r;
FD_ZERO(&r);
FD_SET(sd,&r);
struct timeval t;
t.tv_sec = 5;
t.tv_usec = 0;


rc =  select(sd+1,&r,NULL,NULL,&t);




if(rc>0)
break;
//}
}
rc = UDP_Write(sd, &saddr, (char *) &clientmsg , sizeof(clientmsg));

return 0;
}



int main()
{
int val,i;
MFS_Stat_t *m = malloc(sizeof(MFS_Stat_t));
//char dum[4096] = "test\0";
MFS_Stat_t *n = malloc(sizeof(MFS_Stat_t));
MFS_Stat_t *k = malloc(sizeof(MFS_Stat_t));
char dum[4096] = "test\0";
//char dum[4096] = "test\0";
char data[4096]="START BLOCK 1 \0 END BLOCK 1\0";
char rtdata[4096]="file2";
MFS_Init(dum,6000);
//int r =MFS_Lookup(0,dum);
//printf("inode num %d",r);
//for(i=0;i<896; i++)
MFS_Creat(0,0,dum);
MFS_Creat(1,0,dum);
MFS_Creat(2,1,rtdata);
//MFS_Creat(2,0,rtdata);
//MFS_Write(2,data,0);
//int inum = MFS_Lookup(0,"sss");
//printf("inum = %d\n", inum);
//MFS_Stat(0,m);
//MFS_Stat(inum,n);
//printf("the type returned is %d and size is %d\n",m->type, m->size);
//MFS_Creat(0,1,dum);
//printf("the type returned is %d and size is %d\n",n->type, n->size);
//MFS_Creat(0,1,dum);
//MFS_Write(inum,data,0);
//MFS_Stat(inum,k);

//printf("the type returned is %d and size is %d\n",k->type, k->size);
//MFS_Creat(0,1,dum);
//MFS_Read(1,rtdata,0);
//printf("data read is %s\n", rtdata);
//val = MFS_Lookup(0, dum);
//printf(" inode num = %d \n",val);
MFS_Shutdown();
return 0;
}

