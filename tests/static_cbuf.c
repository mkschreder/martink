#include <stdio.h>
#include "../include/static_cbuf.h"

DECLARE_STATIC_CBUF(inbuf, char, 10L); 

int main(){
	int count = 0;
	#define X 10
	printf("Next pow2 of %d is %d\n", X, NEXT_POW2(X)); 
	printf("Using buffer of size: %d, mask: %x\n", inbuf.total_size, inbuf.size_mask); 
	
	while(1){
		int ret = cbuf_put(&inbuf, 'a' + count); 
		if(ret == -1){
			printf("Buffer full at %d\n", count);
			break; 
		}
		printf("Put at head %d, tail %d\n", inbuf.head, inbuf.tail); 
		count++;
		if(count == 20) exit(0); 
	}
	while(!cbuf_is_empty(&inbuf)){
		printf("Get: %c\n", cbuf_get(&inbuf)); 
	}
	return 0; 
}
		 
	
