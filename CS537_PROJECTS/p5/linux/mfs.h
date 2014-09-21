#ifndef __MFS_h__
#define __MFS_h__

#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)

#define MFS_BLOCK_SIZE   (4096)

typedef struct __MFS_Stat_t {
		  // bytes 
    int type;  // MFS_DIRECTORY or MFS_REGULAR 
    int size;
    // note: no permissions, access times, etc.
} MFS_Stat_t;

typedef struct __MFS_DirEnt_t {
    char name[60];  // up to 60 bytes of name in directory (including \0)
    int  inum;      // inode number of entry (-1 means entry not used)
} MFS_DirEnt_t;

typedef struct __MFS_Checkpt {  // Checkpoint region structure
  int log,InodeMap[256];
}MFS_CheckPt;

typedef struct __MFS_InodeMap {  // Imap structure
    int Inodes[16];
}MFS_InodeMap;

typedef struct __MFS_Inode {    // Inode Structure
  MFS_Stat_t stat;
  int BlockPointer[14];

}MFS_Inode;

typedef struct __MFS_InodeDirEnt {
  MFS_DirEnt_t DirEnt[64];
}MFS_InodeDirEnt;

typedef struct __MFS_Mem_InodeMap {
int allinodes[4096];
}MFS_Mem_InodeMap;

typedef struct __MFS_DataBlock {
char DataBlock[4096];
}MFS_DataBlock;

//Client message format
typedef struct __MFS_Client_Msg {
int pinum,inum,block_offset,msg_type, port, file_type,ret_val;
MFS_Stat_t stat;
char name[60];
char buffer[4096];
}MFS_Client_Msg;

#define LOOKUP 1
#define STAT 2
#define WRITE 3
#define READ 4
#define CREAT 5
#define UNLINK 6
#define SHUTDOWN 7

int MFS_Init(char *hostname, int port);
int MFS_Lookup(int pinum, char *name);
int MFS_Stat(int inum, MFS_Stat_t *m);
int MFS_Write(int inum, char *buffer, int block);
int MFS_Read(int inum, char *buffer, int block);
int MFS_Creat(int pinum, int type, char *name);
int MFS_Unlink(int pinum, char *name);
int MFS_Shutdown();

#endif // __MFS_h__
