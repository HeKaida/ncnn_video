#ifndef _SERIAL_H
#define _SERIAL_H

 
int Serial_Init(const char *device, int baudrate);
int Serial_Close(int fd);
int Serial_Send(int fd, const unsigned char *buf, int len);
int Serial_Read(int fd, unsigned char *buf, int len);


#endif
