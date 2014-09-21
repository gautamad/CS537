#include <stdio.h>
#include "udp.h"
#include "mfs.h"
#define BUFFER_SIZE (4096)

void Server_Diskcreate();
void Server_Diskread();
int Server_Lookup(int,char *);
int Server_Creat(int pinum, int type,char* name);
int Server_Write(int inum, char *buffer, int block);
int Server_Read(int inum, char *buffer, int block);
int Server_Shutdown();
int Server_Unlink(int pinum,char *name);
int Server_Stat(int inum, MFS_Stat_t *m);

MFS_Client_Msg servermsg;

MFS_CheckPt checkpt;
MFS_InodeMap inodemap;

MFS_Inode rootinode;
MFS_Mem_InodeMap meminodemap;

int fd;
char name[10]="lol\0";
char buffer[4096] ="HEHEHEHEHHEHEHE\0";
char temp_buffer[4096];
int main(int argc, char *argv[]) {
    int port; 
   printf("Server running \n"); 
  // MFS_Stat_t *m = (MFS_Stat_t *)malloc(sizeof(MFS_Stat_t));
    MFS_Client_Msg clientmsg;
    port = atoi(argv[1]);
    printf("The port number is %d\n", port);
    fd = open(argv[2], O_RDWR);
    if(fd < 0)
      {
    	fd = open(argv[2], O_CREAT|O_RDWR|O_TRUNC, S_IRWXU);
    	if(fd < 0)
	  {
	    printf("Cannot open or create the file");
	  }
	else {
	  Server_Diskcreate();
 	//  Server_Creat(0,1,"test");
 	}
      }
    else
      Server_Diskread();
    
   // Server_Creat(0,1,name);
    //Server_Unlink(0,name);
    //Server_Write(1,buffer,0);   

	
	int sd = UDP_Open(port);
      assert(sd > -1);
    
	printf(" SERVER:: waiting in loop\n");
	
    while (1) {
    struct sockaddr_in s;

    int rc = UDP_Read(sd, &s, (char *)&clientmsg,sizeof(MFS_Client_Msg));
    if(rc > 0) {
//    printf("Received a msg %d\n", clientmsg.msg_type);

/*********** LOOKUP REQUEST *************/

    if(clientmsg.msg_type == 1)
	{
	int rc;
         rc = Server_Lookup(clientmsg.pinum, clientmsg.name);
         if(rc == -1)
	 {
	  servermsg.ret_val = -1;
	 }
	 else
	 {
	  servermsg.ret_val = rc;
	 }
	  servermsg.inum = rc;
          UDP_Write(sd, &s,(char*) &servermsg,sizeof(servermsg)); 
	}
	
/************* STAT REQUEST ***********i*****/
    else if(clientmsg.msg_type == 2)
	{
          int rc;
          rc = Server_Stat(clientmsg.inum, &(servermsg.stat));
          servermsg.ret_val = rc;
	  UDP_Write(sd, &s,(char*) &servermsg,sizeof(servermsg)); 
        }
/************* WRITE REQUEST ****************/

    else if(clientmsg.msg_type == 3)
	{
  	int rc;
	rc = Server_Write(clientmsg.inum, clientmsg.buffer, clientmsg.block_offset);
        servermsg.ret_val = rc;
        UDP_Write(sd,&s, (char *) &servermsg, sizeof(servermsg));
        }
/************** READ REQUEST ***************/        
    else if(clientmsg.msg_type == 4)
	{
	int rc;
	rc = Server_Read(clientmsg.inum,temp_buffer,clientmsg.block_offset);
        servermsg.ret_val = rc;
        memcpy(servermsg.buffer,temp_buffer,4096);
	UDP_Write(sd, &s, (char*) &servermsg, sizeof(servermsg));
	}
/*************** CREATE REQUEST ***********/

    else if(clientmsg.msg_type == 5)
       {
	int rc;
	rc = Server_Creat(clientmsg.pinum,clientmsg.file_type,clientmsg.name);
        servermsg.ret_val = rc;
        UDP_Write(sd, &s, (char*) &servermsg, sizeof(servermsg));
       
 	}
    else if(clientmsg.msg_type == 6)
	{
	int rc;
	rc = Server_Unlink(clientmsg.pinum,clientmsg.name);
	servermsg.ret_val = rc;
	UDP_Write(sd, &s, (char*) &servermsg, sizeof(servermsg));	       
	}
    else if(clientmsg.msg_type == 7)

        Server_Shutdown();
    
    }
}
      
    return 0;
}

void printdisk()
{
int i, temp;
lseek(fd,0,0);
for(i=0;i<checkpt.log;i=i+4)
{
read(fd,&temp,4);
printf("value at loc : %d is %d::",i,temp);
}
}
/***********************************INITIAL  FUNCTIONS***********************************/
//Read all disk file pointers
void Server_Diskread()
{
  int count=0,i,j=0;
  
  lseek(fd,0,SEEK_SET);
  read(fd, &checkpt, sizeof(checkpt));

  for(i=0; i<256; i++)
    {
     if(checkpt.InodeMap[i] >= 0)
       {
	 lseek(fd,checkpt.InodeMap[i],0);
	 read(fd,&inodemap,sizeof(MFS_InodeMap));
	 for(j = 0; j<16; j++)
	   {
	     if(inodemap.Inodes[j])
	       {
		 meminodemap.allinodes[count] = inodemap.Inodes[j];
		 count++;
	       }
	   }
       }
    }
}

//Initialize the file system

void Server_Diskcreate() 
{
  int i; 
  
  
  for(i = 0; i < 256; i++) 
    {
      checkpt.InodeMap[i] = -1;
    }
   
  for(i=0; i< 4096;i++)
    meminodemap.allinodes[i]  = -1 ;


  for(i = 0; i< 14; i++)
    {
      rootinode.BlockPointer[i] = -1;
    }

 for(i= 0; i < 16; i++) 
    {
      inodemap.Inodes[i] = -1;
    }


 
 MFS_InodeDirEnt direntblock;
  for(i = 0; i < 64; i++)
    {
  	strcpy(direntblock.DirEnt[i].name,"\0");
	direntblock.DirEnt[i].inum = -1;
    }
 
  strcpy(direntblock.DirEnt[0].name,".\0");
  strcpy(direntblock.DirEnt[1].name,"..\0");
  
  direntblock.DirEnt[0].inum = 0;
  direntblock.DirEnt[1].inum = 0;
  
  lseek(fd, 0, 0);
  checkpt.InodeMap[0] = sizeof(checkpt) + sizeof(direntblock) + sizeof(rootinode);
  checkpt.log = sizeof(checkpt) + sizeof(direntblock) + sizeof(rootinode)  + sizeof(inodemap);
  write(fd, &checkpt, sizeof(checkpt)); //write checkpt first
  printf(" log end = %d \n", checkpt.log);
  write(fd, &direntblock, sizeof(direntblock)); // write directory
  
  rootinode.stat.type = 4096; // notice written differently
  rootinode.stat.size = 0;
  rootinode.BlockPointer[0] = sizeof(checkpt);
  
  write(fd, &rootinode, sizeof(rootinode)); //write 

  inodemap.Inodes[0] = sizeof(checkpt) + sizeof(direntblock);
  write(fd, &inodemap, sizeof(MFS_InodeMap));
//printdisk();
fsync(fd);
}

int newinodenum;


/********************************* FUNCTIONS FOR CLIENT REQUEST *************************************/

int Server_Lookup(int pinum, char *name)
{
int j;
if(pinum > 4095 || pinum <0)
return -1;

if(strlen(name) >60 || strlen(name) <1)
return -1;


int buf,i;
Server_Diskread();
buf = meminodemap.allinodes[pinum];

if(buf == -1)
return -1;

lseek(fd,buf,0);
MFS_Inode inode; 
MFS_InodeDirEnt inodedirent;
read(fd,&inode,sizeof(inode));
if(inode.stat.size != 0)
return -1;
for(j=0;j<14;j++)
{
lseek(fd,inode.BlockPointer[j],0);
read(fd,&inodedirent, sizeof(inodedirent));
for(i=0;i<64;i++)
{
if(!strcmp(inodedirent.DirEnt[i].name, name))
{
return inodedirent.DirEnt[i].inum;
}
}
}
return -1;
}
int watch; 

 int Server_Creat(int pinum, int type, char *name)
 {
int countdirent = 0, imapno, imapoffset;
if((type != 0 && type != 1) || pinum > 4095 || pinum < 0)
return -1;

if(strlen(name) >60 || strlen(name) <1)
return -1;

   imapno = pinum/16;
   imapoffset = pinum % 16;
   int buf, chkptendlog,pinodedirent,i;
   Server_Diskread();
   buf = meminodemap.allinodes[pinum];
   if(meminodemap.allinodes[pinum] == -1)
   return -1;
   MFS_Inode pinode;
   
   lseek(fd, buf, 0);
   read(fd, &pinode, sizeof(pinode));
   if(pinode.stat.size != 0)
   return -1;
   if(pinode.stat.type >= (4096*14))
   return -1;
   chkptendlog = checkpt.log;
       
 int k;      
 MFS_Inode newinode;
 MFS_InodeMap inodemapnew;
 MFS_InodeMap pinodemap,tempinodemap,inodemaptemp;
int imappieceptr, imapnewptr,newimapoffset;
int newimapwrite=0,counting=0;
int freeimap=0,countdir=0,imapaddr =0;
MFS_InodeDirEnt mfsinodedirent,checkdirent;
MFS_InodeDirEnt mfsinodedirentnew;
for(i=0;i<256;i++) {
if(checkpt.InodeMap[i] == -1){
freeimap = i; break;
}
}
if(type == 1)
      { 
       for(i=0; i < 4096;i++)
	 {
	   if(meminodemap.allinodes[i] ==-1)
	     {
	       newinodenum = i;
	       break;
	     }
	 }
       for(i=0;i<14;i++)
       {
	if(pinode.BlockPointer[i] == -1) 
	{
	  countdir++;
	break;
        }
      }
 	if(countdir == 0) // the directory is full
	{
	lseek(fd,pinode.BlockPointer[13],0);
	read(fd,&checkdirent,sizeof(checkdirent));
	if(checkdirent.DirEnt[63].inum != -1)
	{
	//printf("dir last inum = %d\n", checkdirent.DirEnt[0].inum);
	return -1;
	}
	}
for(k=0;k<14;k++)
{
if(pinode.BlockPointer[k] == -1){
       pinodedirent = pinode.BlockPointer[k-1];
       //printf("value = %d\n",k);
	break;
}
else {
pinodedirent = pinode.BlockPointer[k];
}
}
     
	 lseek(fd,pinodedirent,0);
       
       read(fd, &mfsinodedirent, sizeof(mfsinodedirent));
       
       for(i=0; i<64; i++)
	 {
	   if(mfsinodedirent.DirEnt[i].inum == -1)
	     {
	       countdirent++;
	       strcpy(mfsinodedirent.DirEnt[i].name,name);
	       mfsinodedirent.DirEnt[i].inum = newinodenum;
	       break;
	     }
	 }
       
	if(countdirent==0)
	{
	for(i=0;i<64;i++)
	mfsinodedirentnew.DirEnt[i].inum = -1;
		mfsinodedirentnew.DirEnt[0].inum = newinodenum;
 		strcpy(mfsinodedirentnew.DirEnt[0].name, name);
	for(i=0;i<14;i++)
       {
	if(pinode.BlockPointer[i] == -1) 
	{
	  pinode.BlockPointer[i] = chkptendlog;
	  break; 
 	}
       }
       }
       else
	{
	pinode.BlockPointer[k-1]=chkptendlog;
	}
      //int imappieceptr, imapnewptr, newimapoffset;
      imappieceptr = newinodenum % 16;
       imapaddr = checkpt.InodeMap[imapno];
       
if(imappieceptr == 0)	
newimapwrite = 1;
else
newimapwrite = 0;
       
	       lseek(fd,imapaddr,0);
	       read(fd, &pinodemap, sizeof(pinodemap));
	       
	       /*for(i=0; i<16;i++)
	       {
		 if(pinodemap.Inodes[i] < 1)
		   {
		     pinodemap.Inodes[i] = chkptendlog + sizeof(MFS_InodeDirEnt) + sizeof(MFS_Inode);
		     break;
		   }
	       }*/
newimapoffset = newinodenum % 16; 
	pinodemap.Inodes[imapoffset]= chkptendlog + sizeof(MFS_InodeDirEnt);
	imapnewptr = newinodenum / 16;
        lseek(fd,checkpt.InodeMap[imapnewptr],0);
	read(fd,&inodemaptemp, sizeof(inodemaptemp)); 

	if(newimapwrite == 1)
	{
		for(i=0;i<16;i++)
		inodemapnew.Inodes[i] = -1;
		inodemapnew.Inodes[0] = chkptendlog + sizeof(MFS_InodeDirEnt) + sizeof(MFS_Inode);
	}
	else if(imapnewptr != imapno)
	{
	   lseek(fd,checkpt.InodeMap[imapnewptr],0);
inodemaptemp.Inodes[newimapoffset] = chkptendlog+ sizeof(MFS_InodeDirEnt) + sizeof(MFS_Inode);
	}
	else
	{	
	pinodemap.Inodes[newimapoffset] = chkptendlog + sizeof(MFS_InodeDirEnt) + sizeof(MFS_Inode);
	}	
    
	   newinode.stat.type = 0;
	   newinode.stat.size = type;
	   
	   for(i=0;i<14;i++)
	     newinode.BlockPointer[i] = -1;
     if(newimapwrite == 1 || (imapnewptr != imapno))
	 checkpt.log = chkptendlog + sizeof(MFS_InodeDirEnt) + 2*sizeof(MFS_Inode) +2*sizeof(MFS_InodeMap);
     else
	 checkpt.log = chkptendlog + sizeof(MFS_InodeDirEnt) + 2*sizeof(MFS_Inode) +sizeof(MFS_InodeMap);

 	 lseek(fd,chkptendlog,0);
	 if(countdirent == 0)
	  write(fd, &mfsinodedirentnew, sizeof(mfsinodedirentnew));
	 else
	  write(fd, &mfsinodedirent, sizeof(mfsinodedirent));
	 
          write(fd, &pinode, sizeof(pinode));
	  write(fd, &newinode, sizeof(newinode));

	if(newimapwrite == 1)
	{
	write(fd,&pinodemap, sizeof(pinodemap));	
	write(fd, &inodemapnew, sizeof(inodemapnew));
	}
        else if(imapnewptr != imapno)
	{
	write(fd,&pinodemap, sizeof(pinodemap));
	write(fd, &inodemaptemp, sizeof(inodemaptemp));
	}
	else
	write(fd, &pinodemap, sizeof(pinodemap));
 	
	checkpt.InodeMap[imapno] = chkptendlog + sizeof(MFS_InodeDirEnt) + 2*sizeof(MFS_Inode);
	if(newimapwrite	==1)
	checkpt.InodeMap[freeimap] = chkptendlog + sizeof(MFS_InodeDirEnt) + 2*sizeof(MFS_Inode)+sizeof(MFS_InodeMap);
 	else if(imapnewptr != imapno)
	checkpt.InodeMap[imapnewptr] = chkptendlog + sizeof(MFS_InodeDirEnt) + 2*sizeof(MFS_Inode)+sizeof(MFS_InodeMap);

	lseek(fd,0,0);
	write(fd, &checkpt, sizeof(checkpt));
}
else
{
 for(i=0; i < 4096;i++)
	 {
	   if(meminodemap.allinodes[i] ==-1)
	     {
	       newinodenum = i;
	       break;
	     }
	 }
       for(i=0;i<14;i++)
       {
	if(pinode.BlockPointer[i] == -1) 
	{
	  countdir++;
	break;
        }
      }
 	if(countdir == 0) // the directory is full
	{
	lseek(fd,pinode.BlockPointer[13],0);
	read(fd,&checkdirent,sizeof(checkdirent));
	if(checkdirent.DirEnt[63].inum != -1)
	{
	//printf("dir last inum = %d\n", checkdirent.DirEnt[0].inum);
	return -1;
	}
	}
for(k=0;k<14;k++)
{
if(pinode.BlockPointer[k] == -1){
       pinodedirent = pinode.BlockPointer[k-1];
       printf("value = %d\n",k);
	break;
}
else {
pinodedirent = pinode.BlockPointer[k];
}
}
     
	 lseek(fd,pinodedirent,0);
       
       read(fd, &mfsinodedirent, sizeof(mfsinodedirent));
       
       for(i=0; i<64; i++)
	 {
	   if(mfsinodedirent.DirEnt[i].inum == -1)
	     {
	       countdirent++;
	       strcpy(mfsinodedirent.DirEnt[i].name,name);
	       mfsinodedirent.DirEnt[i].inum = newinodenum;
	       break;
	     }
	 }
       
	if(countdirent==0)
	{
	for(i=0;i<64;i++)
	mfsinodedirentnew.DirEnt[i].inum = -1;
		mfsinodedirentnew.DirEnt[0].inum = newinodenum;
 		strcpy(mfsinodedirentnew.DirEnt[0].name, name);
	for(i=0;i<14;i++)
       {
	if(pinode.BlockPointer[i] == -1) 
	{
	  pinode.BlockPointer[i] = chkptendlog;
	  break; 
 	}
       }
       }
       else
	{
	pinode.BlockPointer[k-1]=chkptendlog;
	}
      //int imappieceptr, imapnewptr, newimapoffset;
      imappieceptr = newinodenum % 16;
       imapaddr = checkpt.InodeMap[imapno];
       
if(imappieceptr == 0)	
newimapwrite = 1;
else
newimapwrite = 0;
       
	       lseek(fd,imapaddr,0);
	       read(fd, &pinodemap, sizeof(pinodemap));
	       
	       /*for(i=0; i<16;i++)
	       {
		 if(pinodemap.Inodes[i] < 1)
		   {
		     pinodemap.Inodes[i] = chkptendlog + sizeof(MFS_InodeDirEnt) + sizeof(MFS_Inode);
		     break;
		   }
	       }*/
newimapoffset = newinodenum % 16; 
	pinodemap.Inodes[imapoffset]= chkptendlog + sizeof(MFS_InodeDirEnt);
	imapnewptr = newinodenum / 16;
        lseek(fd,checkpt.InodeMap[imapnewptr],0);
	read(fd,&inodemaptemp, sizeof(inodemaptemp)); 

	if(newimapwrite == 1)
	{
		for(i=0;i<16;i++)
		inodemapnew.Inodes[i] = -1;
		inodemapnew.Inodes[0] = chkptendlog + 2*sizeof(MFS_InodeDirEnt) + sizeof(MFS_Inode);
	}
	else if(imapnewptr != imapno)
	{
	   lseek(fd,checkpt.InodeMap[imapnewptr],0);
inodemaptemp.Inodes[newimapoffset] = chkptendlog+ 2*sizeof(MFS_InodeDirEnt) + sizeof(MFS_Inode);
	}
	else
	{	
	pinodemap.Inodes[newimapoffset] = chkptendlog + 2*sizeof(MFS_InodeDirEnt) + sizeof(MFS_Inode);
	}	
    
	   newinode.stat.type = 4096;
	   newinode.stat.size = type;
	   
	   for(i=0;i<14;i++)
	     newinode.BlockPointer[i] = -1;
 MFS_InodeDirEnt direntblock;
  for(i = 0; i < 64; i++)
    {
  	strcpy(direntblock.DirEnt[i].name,"\0");
	direntblock.DirEnt[i].inum = -1;
    }
 
  strcpy(direntblock.DirEnt[0].name,".\0");
  strcpy(direntblock.DirEnt[1].name,"..\0");
  
  direntblock.DirEnt[0].inum = newinodenum;
  direntblock.DirEnt[1].inum = pinum;
	newinode.BlockPointer[0] = chkptendlog +sizeof(MFS_InodeDirEnt) + sizeof(MFS_Inode);
     if(newimapwrite == 1 || (imapnewptr != imapno))
	 checkpt.log = chkptendlog + 2*sizeof(MFS_InodeDirEnt) + 2*sizeof(MFS_Inode) +2*sizeof(MFS_InodeMap);
     else
	 checkpt.log = chkptendlog + 2*sizeof(MFS_InodeDirEnt) + 2*sizeof(MFS_Inode) +sizeof(MFS_InodeMap);

 	 lseek(fd,chkptendlog,0);
	 if(countdirent == 0)
	  write(fd, &mfsinodedirentnew, sizeof(mfsinodedirentnew));
	 else
	  write(fd, &mfsinodedirent, sizeof(mfsinodedirent));
	 
          write(fd, &pinode, sizeof(pinode));
 	  write(fd, &direntblock, sizeof(direntblock));
	  write(fd, &newinode, sizeof(newinode));

	if(newimapwrite == 1)
	{
	write(fd,&pinodemap, sizeof(pinodemap));	
	write(fd, &inodemapnew, sizeof(inodemapnew));
	}
        else if(imapnewptr != imapno)
	{
	write(fd,&pinodemap, sizeof(pinodemap));
	write(fd, &inodemaptemp, sizeof(inodemaptemp));
	}
	else
	write(fd, &pinodemap, sizeof(pinodemap));
 	
	checkpt.InodeMap[imapno] = chkptendlog + 2*sizeof(MFS_InodeDirEnt) + 2*sizeof(MFS_Inode);
	if(newimapwrite	==1)
	checkpt.InodeMap[freeimap] = chkptendlog + 2*sizeof(MFS_InodeDirEnt) + 2*sizeof(MFS_Inode)+sizeof(MFS_InodeMap);
 	else if(imapnewptr != imapno)
	checkpt.InodeMap[imapnewptr] = chkptendlog + 2*sizeof(MFS_InodeDirEnt) + 2*sizeof(MFS_Inode)+sizeof(MFS_InodeMap);

	lseek(fd,0,0);
	write(fd, &checkpt, sizeof(checkpt));
}
fsync(fd);
return 0;
}

int Server_Write(int inum, char *buffer, int block)
{
if(block<0 ||block >13)
return -1;

printf("inside server write \n");
int buf,endlog,imapno, imapoffset,imaploc;
Server_Diskread();
buf = meminodemap.allinodes[inum];
imapno = inum/16;
imapoffset = inum%16;
MFS_Inode inode;
MFS_InodeMap inodemap;
endlog = checkpt.log;
imaploc = checkpt.InodeMap[imapno];
lseek(fd,imaploc,0);
read(fd, &inodemap, sizeof(inodemap)); // Read Imap into memory
lseek(fd, buf, 0);
read(fd, &inode, sizeof(inode)); // Read Inode into memory
if(inode.stat.size == 0) // Directory and not a file
return -1;
if(inode.BlockPointer[block] == -1)
{
inode.stat.type = (block+1)*MFS_BLOCK_SIZE;// inode.stat.type+MFS_BLOCK_SIZE;
}
inode.BlockPointer[block] = endlog + sizeof(inode);
inodemap.Inodes[imapoffset] = endlog;
lseek(fd,endlog,0);
write(fd, &inode, sizeof(inode));
write(fd, buffer, MFS_BLOCK_SIZE);
write(fd, &inodemap, sizeof(inodemap));
checkpt.InodeMap[imapno] = endlog+sizeof(inode)+MFS_BLOCK_SIZE;
checkpt.log = endlog+sizeof(inode)+MFS_BLOCK_SIZE+sizeof(inodemap);
lseek(fd,0,0);
write(fd, &checkpt, sizeof(checkpt));
fsync(fd);
return 0;
}

int Server_Read(int inum, char *buffer, int block)
{
if(block<0 || block>13)
return -1;
printf("inside server read\n");
int buf;
Server_Diskread();
buf = meminodemap.allinodes[inum];

MFS_Inode inode;
lseek(fd,buf,0);
read(fd,&inode,sizeof(inode));

if(inode.BlockPointer[block] == -1)
return -1;

lseek(fd,inode.BlockPointer[block],0);
read(fd,buffer,MFS_BLOCK_SIZE);
return 0;
}

MFS_Inode inode1;
MFS_InodeDirEnt dirent,inodedirent;
MFS_Inode inode;

int Server_Unlink(int pinum, char *name)
{
if(strlen(name) >60 || strlen(name) <1)
return -1;
int buf,i,count=0;

Server_Diskread();
buf = meminodemap.allinodes[pinum];
lseek(fd,buf,0);
read(fd,&inode,sizeof(inode));
lseek(fd,inode.BlockPointer[0],0);
read(fd, &inodedirent, sizeof(inodedirent));

for(i=0; i<64; i++)
{
if(!strcmp(inodedirent.DirEnt[i].name,name))
{
buf = meminodemap.allinodes[inodedirent.DirEnt[i].inum];
lseek(fd,buf,0);
read(fd,&inode1,sizeof(inode1));
if( (((inode1).stat).size) == 0)
{
printf("type = %d\n", inode1.stat.size);
lseek(fd,inode1.BlockPointer[0],0);
read(fd,&dirent,sizeof(dirent));
if(dirent.DirEnt[2].inum != -1)
return -1;
}
//else

strcpy(inodedirent.DirEnt[i].name,"\0");
inodedirent.DirEnt[i].inum = -1;
count++;
break;

}
}
if(count == 0)
return -1;

lseek(fd,inode.BlockPointer[0],0);
write(fd, &inodedirent, sizeof(inodedirent));

fsync(fd);
return 0;
}

int Server_Shutdown()
{
printf("inside server shutdown \n");
fsync(fd);
close(fd);
exit(0);
return 0;
}

int Server_Stat(int inum, MFS_Stat_t *m)
{
if(inum <0 || inum>4095)
return -1;

printf("inside server stat\n");
int buf;
Server_Diskread();
buf = meminodemap.allinodes[inum];
if(buf == -1)
return -1;

lseek(fd,buf,0);
MFS_Inode inode; 
read(fd,&inode,sizeof(inode));
m->type=inode.stat.size;
m->size=inode.stat.type;
return 0;
}


