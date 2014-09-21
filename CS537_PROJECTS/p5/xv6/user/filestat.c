#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
struct stat x;
int fd;

fd = open(argv[1],O_CREATE | O_RDWR | O_CHECKED);
 fstat(fd, &x);
printf(1,"Type: %d",x.type);
printf(1,"\nDevice number: %d",x.dev);
printf(1,"\nInode number: %d",x.ino);
printf(1,"\nNlink: %d",x.nlink);
printf(1,"\nSize: %d",x.size);
printf(1,"\nChecksum: %c\n",x.checksum);
//return 0;
//printf(1,"%d",fd);
exit();
}
