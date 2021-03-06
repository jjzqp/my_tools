#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#define ntohl(x) (unsigned int)((((unsigned int)(x))>>24) | ((((unsigned int)(x))>>8)&0xff00) | (((unsigned int)(x))<<24) | ((((unsigned int)(x))&0xff00)<<8))
#define ntoh(x) (unsigned hort)((((unsigned int)(x))>>8&0x00ff) | (((unsigned int)(x))<<8&0xff00))

#define htonl(A)  ((((unsigned int)(A) & 0xff000000) >> 24) | \
                  (((unsigned int)(A) & 0x00ff0000) >> 8)  | \
                  (((unsigned int)(A) & 0x0000ff00) << 8)  | \
                  (((unsigned int)(A) & 0x000000ff) << 24))
                  
#define  CHANGE_264_DATA   

int printf_buf(void *buf,int len)
{
	int i = 0 ;
	unsigned char *tmp = (unsigned char*)buf;
	for(i=0;i<len;i++)
		printf("%x ",tmp[i]);
	printf("\n");
	return 0;
}

int change_buf_data(unsigned char *buf)
{
	unsigned int change_data = 0x01000000;
 	unsigned int tmp, tmp2, tmp3;
    if (*((char *)(buf + 4)) == 0x67) {
        memcpy(&tmp, buf, 4);
        tmp = htonl(tmp);
        memcpy(buf, &change_data, 4);
        memcpy(&tmp2, buf + tmp + 4, 4);
        tmp2 = htonl(tmp2);
        memcpy(buf + tmp + 4, &change_data, 4);
        memcpy(buf + tmp + tmp2 + 8, &change_data, 4);
    } else {
        memcpy(buf, &change_data, 4);
    }

	return 0;
}
int main()
{
	#define READ_SIZE    6
	printf("test...\n");
	int ret;
	unsigned char cache[READ_SIZE];
	int sps_len = 0;
	int pps_len = 0;
    int fnum = 0;
    int cur_addr = 0;

	FILE *f = fopen("src.264","rb");
	FILE *f_out = fopen("out.264","wb");
	int offset = 0;
	int cnt = 0;
	//fseek(f,cur_addr,SEEK_CUR);
	unsigned char *buf = (unsigned char *)malloc(1024*1024);
	while(1) {
		ret = fread(cache,READ_SIZE,1,f);
		cur_addr += READ_SIZE;
		fseek(f, cur_addr-READ_SIZE, SEEK_SET);
		cur_addr -= READ_SIZE;
		//printf_buf(cache,READ_SIZE);
		cnt++;
		if(ret == 0){
			printf("end\n");
			goto __END;
			
		}
	
        if ((cache[0] == 0) && (cache[4] == 0x41) && (cache[5] == 0x9a)) {
        	memcpy(&offset, &cache[0], 4);
        	offset = ntohl(offset);
        	printf(" %d | P frame   | %d \n",fnum,offset);
        	//fseek(f, offset-READ_SIZE, SEEK_CUR);
        	ret = fread(buf,offset,1,f);
       		if(ret == 0){
				printf("end\n");
				goto __END;
				
			}
        	cur_addr += offset;
        	//printf("cur_addr==%x\n",cur_addr);
        	change_buf_data(buf);
        	fnum++;
#ifndef CHANGE_264_DATA
        } else if ((cache[0] == 0) && (cache[4] == 0x67)) { 
        	memcpy(&offset, &cache[0], 4);
        	offset = ntohl(offset);
	        printf(" %d | SPS frame | %d\n",fnum,offset);
        	//fseek(f, offset, SEEK_CUR);	
        	offset += 4;
        	ret = fread(buf,offset,1,f);
       		if(ret == 0){
				printf("end\n");
				goto __END;
				
			}
        	cur_addr += offset;
        	printf("cur_addr==%x\n",cur_addr);
        	//change_buf_data(buf);
        	fnum++;
        } else if ((cache[0] == 0) && (cache[4] == 0x68)) { 
        	memcpy(&offset, &cache[0], 4);
        	offset = ntohl(offset);
        	printf(" %d | PPS frame | %d\n",fnum,offset);
        	//fseek(f, offset, SEEK_CUR);
        	offset += 4;
        	ret = fread(buf,offset,1,f);
       		if(ret == 0){
				printf("end\n");
				goto __END;
				
			}
        	cur_addr += offset;
        	printf("cur_addr==%x\n",cur_addr);	
        	
        	//change_buf_data(buf);
        	fnum++;

        } else if ((cache[0] == 0) && (cache[4] == 0x65)) {

       	    memcpy(&offset, &cache[0], 4);
        	offset = ntohl(offset);
        	printf(" %d | IDR frame | %d\n",fnum,offset);
        	//fseek(f, offset, SEEK_CUR);
        	ret = fread(buf,offset,1,f);
       		if(ret == 0){
				printf("end\n");
				goto __END;
				
			}
        	cur_addr += offset;
        	printf("cur_addr==%x\n",cur_addr);
        	//change_buf_data(buf);
        	fnum++;
        }
#else 
        } else if ((cache[0] == 0) && (cache[4] == 0x67)) {
            //sps
            printf("sps\n");
            unsigned int tmp_addr = cur_addr;
            memcpy(&sps_len, &cache[0], 4);
            sps_len = ntohl(sps_len);	
            sps_len += 4;
            ret = fread(buf,sps_len,1,f);
            tmp_addr += sps_len;
            
            ret = fread(cache,READ_SIZE,1,f);
            tmp_addr += READ_SIZE;
            fseek(f, tmp_addr-READ_SIZE, SEEK_SET);
            tmp_addr -= READ_SIZE;
            if ((cache[0] == 0) && (cache[4] == 0x68)) {
                //pps
                printf("pps\n");
                memcpy(&pps_len, &cache[0], 4);
                pps_len = ntohl(pps_len);
                pps_len += 4;
                ret = fread(buf,pps_len,1,f); 
                printf_buf(buf,pps_len);
                tmp_addr += pps_len;
                ret = fread(cache,READ_SIZE,1,f);
                tmp_addr += READ_SIZE;
                fseek(f, tmp_addr-READ_SIZE, SEEK_SET);
                tmp_addr -= READ_SIZE;
                if ((cache[0] == 0) && (cache[4] == 0x65)) {
                    printf("III\n");
                    memcpy(&offset, &cache[0], 4);
                    offset = ntohl(offset);
                    offset = offset + sps_len + pps_len;
                    fseek(f, cur_addr, SEEK_SET);
                    ret = fread(buf,offset,1,f);
                    cur_addr += offset;
                    printf("cur_addr==%x\n",cur_addr);
                       if(ret == 0){
                        printf("end\n");
                        goto __END;
                        
                    }
                }
                change_buf_data(buf);
                
            }
            
            fwrite(buf,offset,1,f_out);
        }

#endif
        }
	
__END:
	if(f){
		fclose(f);
	}
	if(f_out){
		fclose(f_out);
	}
	
	return 0;
}
