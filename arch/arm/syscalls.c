
#include <arch/soc.h>

#include <stdarg.h>
#include <string.h>

#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>

#ifndef STDOUT_USART
#define STDOUT_USART 0
#endif

#ifndef STDERR_USART
#define STDERR_USART 0
#endif

#ifndef STDIN_USART
#define STDIN_USART 0
#endif

#undef errno
extern int errno;

/*
 environ
 A pointer to a list of environment variables and their values.
 For a minimal environment, this empty list is adequate:
 */
char *__env[1] = { 0 };
char **environ = __env;

int _write(int file, char *ptr, int len);

void _exit(int status) {
	_write(1, "exit", 4);
	while (1) {
			;
	}
}

int _close(int file) {
	return -1;
}
/*
 execve
 Transfer control to a new process. Minimal implementation (for a system without processes):
 */
int _execve(char *name, char **argv, char **env) {
	errno = ENOMEM;
	return -1;
}
/*
 fork
 Create a new process. Minimal implementation (for a system without processes):
 */

int _fork(void) {
	errno = EAGAIN;
	return -1;
}
/*
 fstat
 Status of an open file. For consistency with other minimal implementations in these examples,
 all files are regarded as character special devices.
 The `sys/stat.h' header file required is distributed in the `include' subdirectory for this C library.
 */
int _fstat(int file, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}

/*
 getpid
 Process-ID; this is sometimes used to generate strings unlikely to conflict with other processes. Minimal implementation, for a system without processes:
 */

int _getpid(void) {
	return 1;
}

/*
 isatty
 Query whether output stream is a terminal. For consistency with the other minimal implementations,
 */
int _isatty(int file) {
	switch (file){
	case STDOUT_FILENO:
	case STDERR_FILENO:
	case STDIN_FILENO:
			return 1;
	default:
			//errno = ENOTTY;
			errno = EBADF;
			return 0;
	}
}


/*
 kill
 Send a signal. Minimal implementation:
 */
int _kill(int pid, int sig) {
    errno = EINVAL;
    return (-1);
}

/*
 link
 Establish a new name for an existing file. Minimal implementation:
 */

int _link(char *old, char *new) {
    errno = EMLINK;
    return -1;
}

/*
 lseek
 Set position in a file. Minimal implementation:
 */
int _lseek(int file, int ptr, int dir) {
    return 0;
}

/*
 sbrk
 Increase program data space.
 Malloc and related functions depend on this
 */
caddr_t _sbrk(int incr) {
	extern char _ebss; // Defined by the linker
	static char *heap_end = 0;
	char *prev_heap_end;

	if (heap_end == 0) {
			heap_end = &_ebss;
	}
	prev_heap_end = heap_end;

	char * stack = (char*) __get_MSP();
	if (heap_end + incr >  stack)
	{
		 //_write (STDERR_FILENO, "Heap and stack collision\n", 25);
		 errno = ENOMEM;
		 return  (caddr_t) -1;
		 //abort ();
	}

	heap_end += incr;
	return (caddr_t) prev_heap_end;
}

/*
 read
 Read a character to a file. `libc' subroutines will use this system routine for input from all files, including stdin
 Returns -1 on error or blocks until the number of characters have been read.
 */
int _read(int file, char *ptr, int len) {
	int n;
	int num = 0;
	switch (file) {
		case STDIN_FILENO:
			for (n = 0; n < len; n++) {
				uint16_t c = uart0_getc(); 
				if(c == SERIAL_NO_DATA) return -1; 
				*ptr++ = c;
				num++;
			}
			break;
		default:
			errno = EBADF;
			return -1;
	}
	return num;
}

/*
 stat
 Status of a file (by name). Minimal implementation:
 int    _EXFUN(stat,( const char *__path, struct stat *__sbuf ));
 */

int _stat(const char *filepath, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

/*
 times
 Timing information for current process. Minimal implementation:
 */

clock_t _times(struct tms *buf) {
    return -1;
}

/*
 unlink
 Remove a file's directory entry. Minimal implementation:
 */
int _unlink(char *name) {
    errno = ENOENT;
    return -1;
}

/*
 wait
 Wait for a child process. Minimal implementation:
 */
int _wait(int *status) {
    errno = ECHILD;
    return -1;
}

/*
 write
 Write a character to a file. `libc' subroutines will use this system routine for output to all files, including stdout
 Returns -1 on error or number of bytes sent
 */
int _write(int file, char *ptr, int len) {
	static serial_dev_t out = 0, err = 0; 
	
	if(!out)
		out = uart_get_serial_interface(0); 
	if(!err)
		err = uart_get_serial_interface(0); 
		
	switch (file) {
		case STDOUT_FILENO: 
			if(out)
				return serial_putn(out, (uint8_t*)ptr, len); 
			break;
		case STDERR_FILENO: /* stderr */
			if(err)
				return serial_putn(err, (uint8_t*)ptr, len); 
			break;
		default:
			errno = EBADF;
			return -1;
	}
	return len;
}
