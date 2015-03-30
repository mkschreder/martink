#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define assert(c) 
int main(){
	char *tmpname;  
int tmpfile;  
pid_t pid;  
int r;  

tmpname = strdup("/tmp/ls_out_XXXXXX");  
assert(tmpname);  
tmpfile = mkstemp(tmpname);  
assert(tmpfile != 0);  
pid = fork();  
if (pid == 0) { // child process  
    r = dup2(STDOUT_FILENO, tmpfile);  
    assert(r == STDOUT_FILENO);  
    execl("/usr/bin/irssi", "irssi", NULL);  
    assert(0);  
} else if (pid > 0) { // parent  
    waitpid(pid, &r, 0);  
    /* you can mmap(2) tmpfile here, and read from it like it was a memory buffer, or  
     * read and write as normal, mmap() is nicer--the kernel handles the buffering  
     * and other memory management hassles.  
     */  
} else {  
    /* fork() failed, bail violently for this example, you can handle differently as  
     * appropriately.  
     */  
    assert(0);  
}  
// tmpfile is the file descriptor for the ls output.  
}
