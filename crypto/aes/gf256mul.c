unsigned char gf256mul(unsigned char a, unsigned char b, unsigned char reducer){
	unsigned char t,ret=0;
	while(a){
					if(a&1)
									ret ^= b;
					t=a&0x80;
					b<<=1;
					if(t)
									b^=reducer;
					a>>=1;
	}
	return ret;
}
