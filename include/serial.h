#ifndef _SERIAL_H
#define _SERIAL_H

int Serial_Init(const char *device, const char *baudrate);
int Serial_Close(const int fd);
int Serial_Puts(const int fd, const char *s);

#endif
