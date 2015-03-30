#define CONFIG_NATIVE 1

#include <arch/soc.h>
#include <kernel/cbuf.c>
#include <inttypes.h>

int main(){
	struct cbuf buf; 
	uint8_t buffer[32]; 
	uint8_t test_data[sizeof(buffer)]; 
	uint8_t tmp[sizeof(buffer)]; 
	
	uint8_t passed = 0; 
	
	cbuf_init(&buf, buffer, sizeof(buffer)); 
	
	for(int c = 0; c < sizeof(test_data); c++){
		test_data[c] = c; 
		tmp[c] = 0; 
	}
	
	printf("putting %lu bytes: free: %d, waiting: %d\n", sizeof(buffer), cbuf_get_free(&buf), cbuf_get_waiting(&buf)); 
	for(int c = 0; c < (sizeof(buffer)); c++){
		if(!cbuf_put(&buf, c) && (c == (sizeof(buffer) - 1))){
			//printf("failed to put at %d\n", c); 
			passed = 1; 
			break; 
		} 
	}
	
	if(!passed) printf("cbuf_put: fail\n"); 
	passed = 0; 
	
	printf("getting %lu bytes: free: %d, waiting: %d\n", sizeof(buffer), cbuf_get_free(&buf), cbuf_get_waiting(&buf)); 
	
	for(int c = 0; c < (sizeof(buffer)); c++){
		uint16_t ch = cbuf_get(&buf); 
		
		if(ch == CBUF_NO_DATA && (c == (sizeof(buffer) - 1))){
			//printf("failed to get at %d\n", c); 
			passed = 1; 
			break; 
		} 
		if(ch != c){
			printf("wrong data at %d: got %d expected %d\n", c, ch, c); 
		}
	}
	
	if(!passed) printf("cbuf_get: fail\n"); 
	passed = 0; 
	
	printf("bulk putting %lu bytes: free: %d, waiting: %d\n", sizeof(buffer), cbuf_get_free(&buf), cbuf_get_waiting(&buf)); 
	
	printf("wrote %d bytes\n", cbuf_putn(&buf, test_data, sizeof(test_data))); 
	
	printf("bulk reading %lu bytes: free: %d, waiting: %d\n", sizeof(buffer), cbuf_get_free(&buf), cbuf_get_waiting(&buf)); 
	
	printf("read %d bytes\n", cbuf_getn(&buf, tmp, sizeof(tmp))); 
	printf("read %d bytes\n", cbuf_getn(&buf, tmp, sizeof(tmp))); 
	
	printf("comparing data\n"); 
	passed = 1; 
	for(int c = 0; c < sizeof(test_data) - 1; c++){
		if(tmp[c] != test_data[c]) {
			printf("failed at %d: got %d expected %d\n", c, tmp[c], test_data[c]); 
		}
	}
	if(passed) printf("all good\n"); 
	printf("done\n"); 
	
	return 0; 
}
