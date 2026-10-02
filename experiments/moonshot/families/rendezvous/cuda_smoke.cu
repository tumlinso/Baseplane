#include "rendezvous.hpp"
#include "e18_cuda.cuh"
#include <iostream>
using namespace bp_moon;using namespace bp_moon::discovery;
void check(cudaError_t error){if(error!=cudaSuccess)throw std::runtime_error(cudaGetErrorString(error));}
template<class T>struct Device {
    T* p=nullptr;
    explicit Device(std::size_t n){check(cudaMalloc(reinterpret_cast<void**>(&p),n*sizeof(T)));}
    ~Device(){if(p)cudaFree(p);}
    Device(const Device&)=delete;Device& operator=(const Device&)=delete;
};
int main(int argc,char** argv){
    if(argc!=3||std::string(argv[1])!="--run"){std::cerr<<"Explicit assigned device required: --run DEVICE\n";return 2;}
    try{
        check(cudaSetDevice(std::stoi(argv[2])));
        std::string input;u64 state=71;
        for(unsigned i=0;i<400;++i){state=state*6364136223846793005ull+1;input+="ACGT"[(state>>32)&3];}
        Costs cost;auto objects=extract({{PackedFixture(input),{1,7,100,9,Strand::forward,input.size()}}},4,cost);
        Directory directory(objects);PostingHash table(directory,1,32,cost);
        if(table.overflow==0)throw std::runtime_error("fixture did not overflow");
        auto rows=packed_rows(table);std::vector<gpu::Query> queries;
        for(const auto& g:directory.groups)queries.push_back(packed_key(g.key,0));
        queries.push_back(packed_key("AAAAA",0)); // Width participates in exact identity.
        std::vector<unsigned> groups(queries.size());
        Device<gpu::Row> device_rows(rows.size());Device<gpu::Query> device_queries(queries.size());Device<unsigned> device_groups(groups.size());
        check(cudaMemcpy(device_rows.p,rows.data(),rows.size()*sizeof(gpu::Row),cudaMemcpyHostToDevice));
        check(cudaMemcpy(device_queries.p,queries.data(),queries.size()*sizeof(gpu::Query),cudaMemcpyHostToDevice));
        check(gpu::lookup(device_rows.p,unsigned(rows.size()),device_queries.p,unsigned(queries.size()),device_groups.p,nullptr));
        check(cudaMemcpy(groups.data(),device_groups.p,groups.size()*sizeof(unsigned),cudaMemcpyDeviceToHost));
        for(unsigned i=0;i<directory.groups.size();++i)if(groups[i]!=i)throw std::runtime_error("GPU lookup differs from complete CPU directory");
        if(groups.back()!=gpu::absent)throw std::runtime_error("missing key became a match");
        std::cout<<"E18 CUDA groups="<<directory.groups.size()<<" overflow_rows="<<table.overflow<<" queries="<<queries.size()<<" passed\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
