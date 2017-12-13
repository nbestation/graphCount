
#include<cassert>
#include<cstdio>
#include<cstring>
#include<string>
#include<vector>
#include<iostream>
#include<sstream>
#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>

using namespace std;

void processRaw(char* mmap_ptr, size_t begin, size_t last, vector<vector<int> >&num, int P);

int main(){
	
	const int P = 320;
	vector<vector<int> > num(P, vector<int>(P,0));

	string file_name = "temp.txt";
	int raw_fd;
	struct stat sb;
	char* raw_mmap_ptr;
	size_t raw_mmap_length;
	size_t txt_buf_th = 1024 * 1024;
	assert((raw_fd = open(file_name.c_str(),O_RDONLY))!=-1);
	assert(fstat(raw_fd,&sb)!=-1);
	raw_mmap_length = sb.st_size;
	assert((raw_mmap_ptr = (char*)mmap(nullptr,raw_mmap_length,PROT_READ,MAP_SHARED,raw_fd,/*offset=*/0))!=MAP_FAILED);
	size_t last_pos = 0;
    for(size_t i=0;i<raw_mmap_length || last_pos<i;++i){
		if((i-last_pos>=txt_buf_th && (raw_mmap_ptr[i]=='\n'||raw_mmap_ptr[i]=='\r'))||(i>raw_mmap_length)){
			processRaw(raw_mmap_ptr,last_pos,i, num, P);
			last_pos=i+1;		
}
	}
	for (int i=0; i<P; i++){
		for (int j=0; j<P-1; j++)
			cout << num[i][j] << ',';
		cout << num[i][P-1];
		if (i<P-1) cout << endl;
	} 
	return 0;
}

void processRaw(char* mmap_ptr, size_t begin, size_t last, vector<vector<int> > &num, int P){
	char* next_pos = nullptr;
	for (char* ptr=mmap_ptr+begin; ptr<mmap_ptr+last; ++ptr){
		for (;isblank(ptr[0]);++ptr);
		for (;ptr[0]=='#';++ptr){
			for(;ptr[0]!='\n'&&ptr[0]!='\r';++ptr);
			for(;isblank(ptr[0]);++ptr);
		}
		uint64_t src_idx = strtoull(ptr,&next_pos,10);
		if(next_pos==ptr) break;
		uint64_t dst_idx = strtoull(next_pos,&ptr,10);
		if(next_pos==ptr) break;
		for(;isblank(ptr[0]);++ptr);
		num[src_idx%P][dst_idx%P]++;
	}
}
