#include <stdio.h>
#include "udp.h"

char comp[30] = "mumble-10.cs.wisc.edu";
int
main(int argc, char *argv[])
{

MFS_Init(comp, 10002);

return 0;
}
