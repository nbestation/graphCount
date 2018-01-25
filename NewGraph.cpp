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
	
    int opt;
    string file_name;
    int P = 0, Q = 1; // number of intervals and sub-intervals
    string help_string = string("Usage: ")+argv[0]+" -r RawFile -P NumberOfIntervals [-Q NumberOfSubIntervals]";
    while ((opt = getopt(argc, argv, "r:P:Q")) != -1){
        switch  (opt){
            case 'r':
                file_name = optarg;
                break;
            case 'P':
                P = atoll(optarg);
                break;
            case 'r':
                Q = atoll(optarg);
                break;
            default:
                cout << help_string;
                return EXIT_FAILURE;
        }
    }
    if (file_name.empty()||!P){
        cout << help_string;
        return -1;
    }
    
	vector<vector<int> > num(P, vector<int>(P,0));

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
