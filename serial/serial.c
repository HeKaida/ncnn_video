#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <aio.h>

#include "config.h"


int Serial_Init(const char *device, int baudrate)
{
	int fd;
	int status;
	struct termios options;

	speed_t myBaud;

	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
	if(fd == -1)
	{
		DBG_SPRINTF(stderr, "open serial fail!\n");
		return -1;
	}

	tcgetattr(fd, &options);

	switch (baudrate)
  	{
	    case      50:	myBaud =      B50 ; break ;
	    case      75:	myBaud =      B75 ; break ;
	    case     110:	myBaud =     B110 ; break ;
	    case     134:	myBaud =     B134 ; break ;
	    case     150:	myBaud =     B150 ; break ;
	    case     200:	myBaud =     B200 ; break ;
	    case     300:	myBaud =     B300 ; break ;
	    case     600:	myBaud =     B600 ; break ;
	    case    1200:	myBaud =    B1200 ; break ;
	    case    1800:	myBaud =    B1800 ; break ;
	    case    2400:	myBaud =    B2400 ; break ;
	    case    4800:	myBaud =    B4800 ; break ;
	    case    9600:	myBaud =    B9600 ; break ;
	    case   19200:	myBaud =   B19200 ; break ;
	    case   38400:	myBaud =   B38400 ; break ;
	    case   57600:	myBaud =   B57600 ; break ;
	    case  115200:	myBaud =  B115200 ; break ;
	    case  230400:	myBaud =  B230400 ; break ;
	    case  460800:	myBaud =  B460800 ; break ;
	    case  500000:	myBaud =  B500000 ; break ;
	    case  576000:	myBaud =  B576000 ; break ;
	    case  921600:	myBaud =  B921600 ; break ;
	    case 1000000:	myBaud = B1000000 ; break ;
	    case 1152000:	myBaud = B1152000 ; break ;
	    case 1500000:	myBaud = B1500000 ; break ;
	    case 2000000:	myBaud = B2000000 ; break ;
	    case 2500000:	myBaud = B2500000 ; break ;
	    case 3000000:	myBaud = B3000000 ; break ;
	    case 3500000:	myBaud = B3500000 ; break ;
	    case 4000000:	myBaud = B4000000 ; break ;

	    default:
	      return -2 ;
  	}

	cfmakeraw(&options);
	cfsetispeed(&options, myBaud);
	cfsetospeed(&options, myBaud);

	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;

	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 0;
	
	tcsetattr(fd, TCSANOW, &options);

	ioctl(fd, TIOCMGET, &status);
	status |= TIOCM_DTR;
	status |= TIOCM_RTS;
	ioctl(fd, TIOCMSET, &status);

	usleep(10000);

	return fd;
}


int Serial_Close(int fd)
{
	int ret;
	
	ret = close(fd);
	if(ret < 0)
	{
		DBG_SPRINTF(stderr, "close serial fail!\n");
		return -1;
	}

	return 0;
}


int Serial_Send(int fd, const unsigned char *buf, int len)
{
	size_t ret;

	ret = write(fd, buf, len);
	if(ret < 0)
	{
		DBG_SPRINTF(stderr, "write fail!\n");
		return -1;
	}

	return 0;
}


int Serial_Read(int fd, unsigned char *buf, int len)
{
	size_t ret;
	
	ret = read(fd, buf, 1);
	if(ret < 0)
	{
		DBG_SPRINTF(stderr, "read fail!\n");
		return -1;
	}

	return ret;
}
