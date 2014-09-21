#ifndef __REQUEST_H__

//void requestHandle(int fd);
void requestHandle(int fd, char *filename, char *method, int is_static, struct stat sbuf, char *cgiargs);
#endif
