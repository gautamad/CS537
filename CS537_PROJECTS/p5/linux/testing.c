#include<stdio.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>


typedef struct struct_std {
int xxx;
int yyy;
}std;

std std1;
std buf;

void main()
{
char *temp2 = (char *) malloc(20);
int fp; 
char *temp;
char a[5] = {10,20,30,40,50};
temp=a;
//temp1 = & temp;
std1.xxx = 5;
std1.yyy = 25;

fp = open("example.img",O_RDWR);
if(fp == -1)
printf("Cannot open file");
//write(fp,temp,sizeof(a));
//lseek(fp,0,0);
//write(fp,temp,sizeof(a));
lseek(fp,0,0);
read(fp,temp2,20);

printf(" Buffer alues are  %d", temp2);
close(fp);
}

