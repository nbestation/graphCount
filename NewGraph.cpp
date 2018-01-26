#include<cassert>
#include<cstdio>
#include<cstring>
#include<string>
#include<vector>
#include<iostream>
#include<algorithm>
#include<sstream>
#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>

using namespace std;

void processRaw(char* mmap_ptr, size_t begin, size_t last, vector<vector<int> >&num, vector<vector<vector<struct edge> > >&edge_list, int P);
inline double DiffTime(struct timespec time1,struct timespec time2){return time2.tv_sec-time1.tv_sec+(time2.tv_nsec-time1.tv_nsec)*1e-9;}

struct edge{
    uint64_t src;
    uint64_t dst;
//    edge (uint64_t s, uint64_t d) : src(s), dst(d) {}
};

inline bool comp_src(const struct edge &a,const struct edge &b){return a.src>b.src;}
inline bool comp_dst(const struct edge &a,const struct edge &b){return a.dst>b.dst;}


int main(int argc, char* argv[]){
	
    int opt;
    string file_name;
    int P = 0;
    int Q = 0; // number of intervals and sub-intervals
    bool sort_src = false, sort_dst = false;
    string help_string = string("Usage: ")+argv[0]+" -r RawFile -P NumberOfIntervals [-Q NumberOfSubIntervals]\n";
    while ((opt = getopt(argc, argv, "r:P:Q:sd")) != -1){
        switch  (opt){
            case 'r':
                file_name = optarg;
                cout << "Open " << file_name << endl;
                break;
            case 'P':
                P = atoll(optarg);
                cout << "P = " << P << endl;
                break;
            case 'Q':
                Q = atoll(optarg);
                cout << "Q = " << Q << endl;
                break;
            case 's':
                sort_src = true;
                break;
            case 'd':
                sort_dst = true;
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
    vector<vector<vector<struct edge> > > edge_list(P, vector<vector<struct edge> >(P, vector<struct edge>(0)));
    cout<< "1" << endl;
	int raw_fd;
	struct stat sb;
	char* raw_mmap_ptr;
	size_t raw_mmap_length;
	size_t txt_buf_th = 1024 * 1024;
	assert((raw_fd = open(file_name.c_str(),O_RDONLY))!=-1);
	assert(fstat(raw_fd,&sb)!=-1);
	raw_mmap_length = sb.st_size;
    
    struct timespec time0,time1;
    clock_gettime(CLOCK_REALTIME,&time0);

	assert((raw_mmap_ptr = (char*)mmap(nullptr,raw_mmap_length,PROT_READ,MAP_SHARED,raw_fd,/*offset=*/0))!=MAP_FAILED);
	size_t last_pos = 0;
    for(size_t i=0;i<raw_mmap_length || last_pos<i;++i){
		if((i-last_pos>=txt_buf_th && (raw_mmap_ptr[i]=='\n'||raw_mmap_ptr[i]=='\r'))||(i>raw_mmap_length)){
			processRaw(raw_mmap_ptr,last_pos,i, num, edge_list, P);
			last_pos=i+1;		
        }
	}

    if (Q){//ForeGraph
        int K = int(double(P)/double(Q));
        vector<vector<vector<struct edge> > > shuffled_edge_list(K, vector<vector<struct edge> >(P, vector<struct edge>(0)));
        for (int i=0; i<K; i++){
            for (int j=0; j<P; j++){
                //shuffled_edge_list[i][P]
                bool finish = false;
                while (!finish){
                    finish = true;
                    for (int k=i; k<P; k=k+K){
                        if (edge_list[k][j].size()){
                            struct edge temp = edge_list[k][j].back();
                            edge_list[k][j].pop_back();
                            shuffled_edge_list[i][j].push_back(temp);
                            finish = false;
                        }
                        else{
                            struct edge temp;
                            temp.src = 0;
                            temp.dst = 0;
                            shuffled_edge_list[i][j].push_back(temp);
                        }
                    }
                }
            }
        }   
    }

    if (sort_src){
        for (int i=0; i<P; i++){
            for (int j=0; j<P; j++){
                sort(edge_list[i][j].begin(), edge_list[i][j].end(), comp_src);
            }
        }
    }
    else if (sort_dst){
        for (int i=0; i<P; i++){
            for (int j=0; j<P; j++){
                sort(edge_list[i][j].begin(), edge_list[i][j].end(), comp_dst);
            }
        }
    }

    clock_gettime(CLOCK_REALTIME,&time1);
    cout << "Finish pre-processing in " << DiffTime(time0, time1) << "sec." << endl;

	return 0;
}

void processRaw(char* mmap_ptr, size_t begin, size_t last, vector<vector<int> > &num, vector<vector<vector<struct edge> > > &edge_list, int P){
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
        struct edge new_edge;
        new_edge.src = src_idx;
        new_edge.dst = dst_idx;
        edge_list[src_idx%P][dst_idx%P].push_back(new_edge);
	}
}
