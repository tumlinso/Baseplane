#include "kernels.cuh"
#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
using namespace bp_cuda_lab;
static void ck(cudaError_t e){if(e!=cudaSuccess)throw std::runtime_error(cudaGetErrorString(e));}
template<class T> struct Buffer {
    T* p=nullptr;std::size_t n;
    explicit Buffer(std::size_t size):n(size){ck(cudaMalloc(reinterpret_cast<void**>(&p),std::max<std::size_t>(1,n)*sizeof(T)));}
    explicit Buffer(const std::vector<T>& x):Buffer(x.size()){if(n)ck(cudaMemcpy(p,x.data(),n*sizeof(T),cudaMemcpyHostToDevice));}
    ~Buffer(){if(p)cudaFree(p);}Buffer(const Buffer&)=delete;Buffer& operator=(const Buffer&)=delete;
    std::vector<T> read()const{std::vector<T> x(n);if(n)ck(cudaMemcpy(x.data(),p,n*sizeof(T),cudaMemcpyDeviceToHost));return x;}
};
static void near(float a,float b){if(!std::isfinite(a)||!std::isfinite(b)||fabsf(a-b)>4e-5f*(1+fabsf(a)+fabsf(b)))throw std::runtime_error("device/scalar mismatch");}
static void eq(Vec a,Vec b){for(int d=0;d<channels;++d)near(a.x[d],b.x[d]);}
static void eq(Affine a,Affine b){eq(a.a,b.a);eq(a.b,b.b);}
static unsigned blocks(unsigned n){return (n+127)/128;}
static std::vector<std::pair<unsigned,float>> rethread_thresholds(const std::vector<Vec>& x) {
    std::vector<float> scores;scores.reserve(x.size());
    for(const auto& v:x)scores.push_back(std::fmaf(.5f,v.x[1],v.x[0]));
    std::sort(scores.begin(),scores.end());std::vector<std::pair<unsigned,float>> cases;
    for(unsigned pct:{0u,1u,5u,25u,50u,100u}) {
        const unsigned k=static_cast<unsigned>((std::size_t(x.size())*pct+50)/100);float threshold=0.f;
        if(scores.empty())threshold=0.f;
        else if(k==0)threshold=scores.back();
        else if(k>=scores.size())threshold=std::nextafter(scores.front(),-INFINITY);
        else threshold=scores[scores.size()-k-1]+(scores[scores.size()-k]-scores[scores.size()-k-1])*.5f;
        cases.emplace_back(pct,threshold);
    }
    return cases;
}
static unsigned bounded_number(const char* text,unsigned maximum) {
    const std::string value(text);std::size_t used=0;
    if(value.empty() || value[0]=='-')throw std::invalid_argument("expected a nonnegative integer");
    const auto parsed=std::stoull(value,&used);
    if(used!=value.size() || parsed>maximum)throw std::invalid_argument("bounded prototype size exceeded");
    return static_cast<unsigned>(parsed);
}
static void finish(){ck(cudaGetLastError());ck(cudaDeviceSynchronize());}
static void measured(const char* experiment,const char* variant,unsigned n,unsigned iters,const std::function<void()>& fn,const std::string& context=""){
    if(!iters)return;for(int i=0;i<3;++i)fn();finish();
    cudaEvent_t a,b;ck(cudaEventCreate(&a));ck(cudaEventCreate(&b));std::vector<float> samples;
    for(unsigned i=0;i<iters;++i){ck(cudaEventRecord(a));fn();ck(cudaEventRecord(b));ck(cudaEventSynchronize(b));float ms;ck(cudaEventElapsedTime(&ms,a,b));samples.push_back(ms);}
    ck(cudaEventDestroy(a));ck(cudaEventDestroy(b));std::sort(samples.begin(),samples.end());
    std::cout<<"{\"phase\":\"device_resident_pipeline\",\"case\":\""<<experiment<<"\",\"variant\":\""<<variant<<"\",\"n\":"<<n<<",\"iterations\":"<<iters<<",\"median_ms\":"<<samples[samples.size()/2]<<",\"p95_ms\":"<<samples[std::min(samples.size()-1,std::size_t(samples.size()*.95))]<<context<<",\"scalar_checked\":true}\n";
}
struct ScanTree {
    unsigned n,ntiles;Buffer<Affine> prefix,totals;std::unique_ptr<ScanTree> upper;
    explicit ScanTree(unsigned len):n(len),ntiles((len+31)/32),prefix(len),totals(ntiles){if(ntiles>1)upper=std::make_unique<ScanTree>(ntiles);}
    void run(const Affine* in){if(!n)return;scan_tiles<<<blocks(n),128>>>(in,prefix.p,totals.p,n);if(upper){upper->run(totals.p);scan_carry<<<blocks(n),128>>>(prefix.p,upper->prefix.p,n);}}
};
static void test_bitlift(unsigned n,unsigned iters){
    auto h=make_planes(n);auto w=make_weights();Buffer<Planes> in(h);Buffer<Vec> out(n);Buffer<Weights> dw(std::vector<Weights>{w});
    std::vector<Vec> expect;for(auto x:h)expect.push_back(lift_scalar(x,w));
    auto a=[&]{if(n)bitlift_warp<<<blocks(n*32),128>>>(in.p,out.p,dw.p,n);};
    auto b=[&]{if(n)bitlift_thread<<<blocks(n),128>>>(in.p,out.p,dw.p,n);};
    for(auto fn:{std::function<void()>(a),std::function<void()>(b)}){fn();finish();auto got=out.read();for(unsigned i=0;i<n;++i)eq(expect[i],got[i]);}
    // Packed two-bit bases plus validity are adapted on device before either equivalent lift path.
    std::vector<PackedWindow> packed;packed.reserve(n);std::vector<Planes> packed_expect;
    for(const auto& p:h){packed.push_back(pack_window(p));packed_expect.push_back(planes_from_packed(packed.back()));}
    Buffer<PackedWindow> dp(packed);Buffer<Planes> adapted(n);
    auto packed_warp=[&]{if(n){packed_to_planes<<<blocks(n),128>>>(dp.p,adapted.p,n);bitlift_warp<<<blocks(n*32),128>>>(adapted.p,out.p,dw.p,n);}};
    auto packed_thread=[&]{if(n){packed_to_planes<<<blocks(n),128>>>(dp.p,adapted.p,n);bitlift_thread<<<blocks(n),128>>>(adapted.p,out.p,dw.p,n);}};
    for(auto fn:{std::function<void()>(packed_warp),std::function<void()>(packed_thread)}){fn();finish();auto actual=adapted.read();auto lifted=out.read();for(unsigned i=0;i<n;++i){if(actual[i].a!=packed_expect[i].a||actual[i].b!=packed_expect[i].b||actual[i].c!=packed_expect[i].c||actual[i].valid!=packed_expect[i].valid)throw std::runtime_error("packed adapter mismatch");eq(lift_scalar(packed_expect[i],w),lifted[i]);}}
    measured("bitlift","packed_prepare_plus_warp_lop3_popc_fp",n,iters,packed_warp);
    measured("bitlift","packed_prepare_plus_thread_bitset_fp",n,iters,packed_thread);
    // Threshold lower-level floats into planes, lift, then repeat once at a coarser level.
    auto lower=make_values(n);std::vector<unsigned> ids(n),ends(n);for(unsigned i=0;i<n;++i){ids[i]=i;ends[i]=i+1;}
    Buffer<Vec> dlower(lower);Buffer<unsigned> dids(ids),dends(ends);unsigned windows=(n+31)/32;
    Buffer<Planes> low_planes(windows);Buffer<unsigned> begins(windows),source_ends(windows);Buffer<Vec> lifted(windows);
    auto threshold_reference=[&](const std::vector<Vec>& values){std::vector<Planes> ps((values.size()+31)/32);std::vector<unsigned> sb(ps.size()),se(ps.size());
        for(unsigned t=0;t<ps.size();++t){unsigned first=t*32,last=std::min(first+32,unsigned(values.size()));sb[t]=first;se[t]=last;for(unsigned j=first;j<last;++j){unsigned bit=1u<<(j-first);if(values[j].x[0]>0.f)ps[t].a|=bit;if(values[j].x[1]>0.f)ps[t].b|=bit;if(values[j].x[2]>0.f)ps[t].c|=bit;ps[t].valid|=bit;}}return std::tuple{ps,sb,se};};
    auto [ref_planes,ref_begin,ref_end]=threshold_reference(lower);std::vector<Vec> ref_lift;for(auto p:ref_planes)ref_lift.push_back(lift_scalar(p,w));
    auto first_adapter=[&]{if(windows)threshold_records_to_planes<<<blocks(windows),128>>>(dlower.p,n,dids.p,dends.p,low_planes.p,begins.p,source_ends.p,windows,0.f,0.f,0.f);};
    first_adapter();finish();auto got_planes=low_planes.read();auto got_begin=begins.read();auto got_end=source_ends.read();for(unsigned i=0;i<windows;++i){if(got_planes[i].a!=ref_planes[i].a||got_planes[i].b!=ref_planes[i].b||got_planes[i].c!=ref_planes[i].c||got_planes[i].valid!=ref_planes[i].valid||got_begin[i]!=ref_begin[i]||got_end[i]!=ref_end[i])throw std::runtime_error("float adapter/source map mismatch");}
    if(windows)bitlift_thread<<<blocks(windows),128>>>(low_planes.p,lifted.p,dw.p,windows);finish();auto first_lift=lifted.read();for(unsigned i=0;i<windows;++i)eq(ref_lift[i],first_lift[i]);
    unsigned coarse_n=(windows+31)/32;Buffer<Planes> coarse_planes(coarse_n);Buffer<unsigned> coarse_begin(coarse_n),coarse_end(coarse_n);Buffer<Vec> coarse_lift(coarse_n);
    std::vector<unsigned> coarse_ids(windows),coarse_ids_end(windows);for(unsigned i=0;i<windows;++i){coarse_ids[i]=i;coarse_ids_end[i]=i+1;}Buffer<unsigned> dcb(coarse_ids),dce(coarse_ids_end);
    auto [coarse_ref,coarse_rb,coarse_re]=threshold_reference(ref_lift);
    if(coarse_n)threshold_records_to_planes<<<blocks(coarse_n),128>>>(lifted.p,windows,dcb.p,dce.p,coarse_planes.p,coarse_begin.p,coarse_end.p,coarse_n,0.f,0.f,0.f);
    if(coarse_n)bitlift_warp<<<blocks(coarse_n*32),128>>>(coarse_planes.p,coarse_lift.p,dw.p,coarse_n);finish();
    auto cp=coarse_planes.read();auto cb=coarse_begin.read();auto ce=coarse_end.read();auto cl=coarse_lift.read();
    for(unsigned i=0;i<coarse_n;++i){if(cp[i].a!=coarse_ref[i].a||cp[i].b!=coarse_ref[i].b||cp[i].c!=coarse_ref[i].c||cp[i].valid!=coarse_ref[i].valid||cb[i]!=coarse_rb[i]||ce[i]!=coarse_re[i])throw std::runtime_error("coarse float source map mismatch");eq(lift_scalar(coarse_ref[i],w),cl[i]);}
}
static void test_scan(unsigned n,unsigned iters){
    auto values=make_values(n);auto input=make_transforms(values,.6f);auto expect=scan_scalar(input);
    Buffer<Vec> dv(values);Buffer<Affine> din(n),custom(n),cub_out(n);ScanTree tree(n);
    std::size_t cub_bytes=0;if(n)ck(cub::DeviceScan::InclusiveScan(nullptr,cub_bytes,din.p,cub_out.p,AffineCompose{},n));
    Buffer<unsigned char> cub_storage(cub_bytes);
    auto produce=[&]{if(n)produce_affine<<<blocks(n),128>>>(dv.p,din.p,n,.6f);};
    auto custom_run=[&]{produce();tree.run(din.p);};
    auto cub_run=[&]{produce();if(n){std::size_t bytes=cub_bytes;ck(cub::DeviceScan::InclusiveScan(cub_storage.p,bytes,din.p,cub_out.p,AffineCompose{},n));}};
    produce();finish();auto generated=din.read();for(unsigned i=0;i<n;++i)eq(input[i],generated[i]);
    custom_run();finish();auto got=tree.prefix.read();for(unsigned i=0;i<n;++i)eq(expect[i],got[i]);
    cub_run();finish();auto cub_values=cub_out.read();for(unsigned i=0;i<n;++i)eq(expect[i],cub_values[i]);
    measured("scan","multilevel_affine_shuffle_scan_with_producer",n,iters,custom_run);
    measured("scan","cub_affine_inclusive_scan_with_producer",n,iters,cub_run);
    // Each four-record summary preserves its exact fine-coordinate source interval.
    if(n){unsigned groups=(n+3)/4;Buffer<Affine> coarse(groups),coarse_prefix(groups);Buffer<unsigned> begins(groups),ends(groups);ScanTree coarse_tree(groups);
        summarize_affine_groups<<<blocks(groups),128>>>(din.p,coarse.p,begins.p,ends.p,n,groups);finish();
        std::vector<Affine> reference(groups);for(unsigned g=0;g<groups;++g){reference[g]=identity();for(unsigned i=4*g;i<std::min(4*g+4,n);++i)reference[g]=compose(reference[g],input[i]);}
        auto cv=coarse.read();auto rb=begins.read();auto re=ends.read();for(unsigned g=0;g<groups;++g){eq(reference[g],cv[g]);if(rb[g]!=4*g||re[g]!=std::min(4*g+4,n))throw std::runtime_error("coarse scan source trace mismatch");}
        coarse_tree.run(coarse.p);finish();auto cp=coarse_tree.prefix.read();auto ce=scan_scalar(reference);for(unsigned g=0;g<groups;++g)eq(ce[g],cp[g]);
    }
}
static void test_rethread(unsigned n,unsigned iters){
    auto x=make_values(n);auto w=make_weights();Buffer<Vec> in(x);Buffer<Weights> dw(std::vector<Weights>{w});
    Buffer<float> out(std::size_t(n)*hidden);Buffer<unsigned> queue(n),count(1),dropped(1);
    std::size_t cub_bytes=0;if(n)ck(select_cub(nullptr,cub_bytes,in.p,n,queue.p,count.p,0.f));
    Buffer<unsigned char> cub_storage(cub_bytes);
    int dev;cudaDeviceProp prop;ck(cudaGetDevice(&dev));ck(cudaGetDeviceProperties(&prop,dev));
    unsigned resident=std::max(1,prop.multiProcessorCount*2);
    for(const auto test_case:rethread_thresholds(x)){
        const auto target_pct=test_case.first;const auto threshold=test_case.second;
        auto expect=refine_scalar(x,w,threshold);
        auto compact=[&]{ck(cudaMemsetAsync(count.p,0,sizeof(unsigned)));ck(cudaMemsetAsync(dropped.p,0,sizeof(unsigned)));if(n){cheap_all<<<blocks(n),128>>>(in.p,out.p,n);select_queue<<<blocks(n),128>>>(in.p,n,threshold,queue.p,n,count.p,dropped.p);refine_queue<<<resident,128>>>(in.p,queue.p,count.p,n,dw.p,out.p);}};
        auto warp=[&]{if(n){cheap_all<<<blocks(n),128>>>(in.p,out.p,n);refine_warp_direct<<<resident,128>>>(in.p,n,threshold,dw.p,out.p);}};
        auto thread=[&]{if(n)refine_thread_direct<<<blocks(n),128>>>(in.p,n,threshold,dw.p,out.p);};
        auto cub=[&]{if(n){cheap_all<<<blocks(n),128>>>(in.p,out.p,n);std::size_t bytes=cub_bytes;ck(select_cub(cub_storage.p,bytes,in.p,n,queue.p,count.p,threshold));refine_queue<<<resident,128>>>(in.p,queue.p,count.p,n,dw.p,out.p);}};
        for(auto fn:{std::function<void()>(compact),std::function<void()>(warp),std::function<void()>(thread),std::function<void()>(cub)}){fn();finish();auto y=out.read();for(std::size_t i=0;i<y.size();++i)near(expect[i],y[i]);}
        compact();finish();unsigned expected=0;for(auto v:x)expected+=select(v,threshold);if(count.read()[0]!=expected||dropped.read()[0]!=0)throw std::runtime_error("queue count mismatch");
        // Each timing record includes its own density context.
        std::cout<<"{\"case\":\"rethread\",\"n\":"<<n<<",\"target_density_pct\":"<<target_pct<<",\"threshold\":"<<threshold<<",\"selected\":"<<expected<<"}\n";
        const auto context=",\"target_density_pct\":"+std::to_string(target_pct)+",\"threshold\":"+std::to_string(threshold)+",\"selected\":"+std::to_string(expected);
        measured("rethread","compacted_warp_mlp",n,iters,compact,context);measured("rethread","uncompacted_warp_mlp",n,iters,warp,context);measured("rethread","thread_mlp",n,iters,thread,context);measured("rethread","cub_stable_select_warp_mlp",n,iters,cub,context);
    }
    // Coarsen floating fine records, then repeat the same select/refine path.
    // A reverse source map checks that compact queue position never becomes identity.
    if(n){unsigned cn=(n+3)/4;Buffer<Vec> coarse(cn);Buffer<unsigned> sources(cn),cq(cn),cc(1);Buffer<float> cout(std::size_t(cn)*hidden);
        auto fine=refine_scalar(x,w,-2.f);refine_thread_direct<<<blocks(n),128>>>(in.p,n,-2.f,dw.p,out.p);
        coarsen_float_records<<<blocks(cn),128>>>(out.p,n,coarse.p,sources.p,cn);finish();
        std::vector<Vec> expected_coarse(cn);std::vector<unsigned> source(cn);for(unsigned g=0;g<cn;++g){source[g]=cn-1-g;unsigned end=std::min(4*g+4,n);for(int d=0;d<channels;++d){float sum=0;for(unsigned i=4*g;i<end;++i)sum+=fine[std::size_t(i)*hidden+d];expected_coarse[g].x[d]=sum/float(end-4*g);}}
        auto cv=coarse.read();for(unsigned i=0;i<cn;++i)eq(cv[i],expected_coarse[i]);
        std::vector<float> expected(std::size_t(cn)*hidden);for(unsigned i=0;i<cn;++i){auto refined=refine_scalar(std::vector<Vec>{expected_coarse[i]},w,0.f);for(int d=0;d<hidden;++d)expected[std::size_t(source[i])*hidden+d]=refined[d];}
        auto coarse_run=[&]{ck(cudaMemsetAsync(cc.p,0,sizeof(unsigned)));ck(cudaMemsetAsync(dropped.p,0,sizeof(unsigned)));cheap_all_sources<<<blocks(cn),128>>>(coarse.p,sources.p,cout.p,cn);select_queue<<<blocks(cn),128>>>(coarse.p,cn,0.f,cq.p,cn,cc.p,dropped.p);refine_queue_sources<<<resident,128>>>(coarse.p,cq.p,cc.p,cn,sources.p,dw.p,cout.p);};
        coarse_run();finish();auto got=cout.read();for(std::size_t i=0;i<got.size();++i)near(expected[i],got[i]);
        std::cout<<"{\"case\":\"rethread_two_level\",\"fine_records\":"<<n<<",\"coarse_records\":"<<cn<<",\"source_identity\":\"reverse_map_preserved\"}\n";
    }
    // Explicit overflow test. Never consume a truncated queue as a successful model result.
    if(n){unsigned cap=n/2;ck(cudaMemset(count.p,0,4));ck(cudaMemset(dropped.p,0,4));select_queue<<<blocks(n),128>>>(in.p,n,-2.f,queue.p,cap,count.p,dropped.p);finish();if(count.read()[0]!=n||dropped.read()[0]!=n-cap)throw std::runtime_error("overflow not accounted");}
}
static void test_rendezvous(unsigned n,unsigned iters){
    auto x=make_values(n);Buffer<Vec> in(x);Buffer<Match> out(n);
    for(int mode=0;mode<3;++mode){
        std::vector<unsigned> key(n);for(unsigned i=0;i<n;++i)key[i]=mode==0?route_key(x[i]):mode==1?0xffffffffu:i;
        auto expect=match_scalar(x,key);Buffer<unsigned> dk(key);
        auto a=[&]{if(n)rendezvous<<<blocks(n),128>>>(in.p,dk.p,out.p,n);};
        auto b=[&]{if(n)rendezvous_pairwise<<<blocks(n),128>>>(in.p,dk.p,out.p,n);};
        for(auto fn:{std::function<void()>(a),std::function<void()>(b)}){fn();finish();auto got=out.read();for(unsigned i=0;i<n;++i){if(got[i].peers!=expect[i].peers||got[i].leader!=expect[i].leader)throw std::runtime_error("peer identity mismatch");eq(got[i].mean,expect[i].mean);}}
        std::cout<<"{\"case\":\"rendezvous\",\"n\":"<<n<<",\"key_mode\":"<<mode<<"}\n";
        const auto context=",\"key_mode\":"+std::to_string(mode);
        measured("rendezvous","match_any_peer_shuffle",n,iters,a,context);measured("rendezvous","pairwise_shuffle",n,iters,b,context);
    }
    // Upper-bound fixture: deliberately place distant source ids in one bounded 128-candidate packet.
    unsigned m=std::min(n,128u);if(m){auto sx=make_values(m);std::vector<unsigned> sk(m),source(m),order(m);
        for(unsigned i=0;i<m;++i){sk[i]=i;source[i]=1000000u+i*100003u;order[i]=i;}
        if(m>32){sk[0]=0xfffffff0u;sk[32]=0xfffffff0u;}if(m>96)for(unsigned i=64;i<96;++i)sk[i]=0x55u;
        auto all_ref=match_supertile_reference(sx,sk);auto local_ref=match_scalar(sx,sk);
        unsigned total_partners=0,local_missed=0;for(unsigned i=0;i<m;++i){total_partners+=all_ref[i].count-1;local_missed+=all_ref[i].count-1-(__builtin_popcount(local_ref[i].peers)-1);}
        Buffer<Vec> dx(sx),sorted_x(m);Buffer<unsigned> dk(sk),dorder(order),sorted_k(m),sorted_order(m),dsource(source);
        Buffer<Match> sorted_matches(m),scattered(m);Buffer<SuperMatch> all_device(m);
        std::size_t temp_bytes=0;ck(cub::DeviceRadixSort::SortPairs(nullptr,temp_bytes,dk.p,sorted_k.p,dorder.p,sorted_order.p,m));Buffer<unsigned char> temp(temp_bytes);
        auto allpairs=[&]{supertile_all_pairs<<<blocks(m),128>>>(dx.p,dk.p,all_device.p,m);};
        auto regroup=[&]{std::size_t bytes=temp_bytes;ck(cub::DeviceRadixSort::SortPairs(temp.p,bytes,dk.p,sorted_k.p,dorder.p,sorted_order.p,m));gather_candidates<<<blocks(m),128>>>(dx.p,dk.p,sorted_order.p,sorted_x.p,sorted_k.p,m);rendezvous<<<blocks(m),128>>>(sorted_x.p,sorted_k.p,sorted_matches.p,m);scatter_matches<<<blocks(m),128>>>(sorted_matches.p,sorted_order.p,scattered.p,m);};
        auto direct=[&]{rendezvous<<<blocks(m),128>>>(dx.p,dk.p,scattered.p,m);};
        allpairs();finish();auto all_got=all_device.read();for(unsigned i=0;i<m;++i){if(all_got[i].count!=all_ref[i].count||all_got[i].leader!=all_ref[i].leader)throw std::runtime_error("supertile all-pairs mismatch");eq(all_got[i].mean,all_ref[i].mean);}
        direct();finish();auto direct_got=scattered.read();for(unsigned i=0;i<m;++i){if(direct_got[i].peers!=local_ref[i].peers)throw std::runtime_error("no-regroup reference mismatch");}
        std::stable_sort(order.begin(),order.end(),[&](unsigned a,unsigned b){return sk[a]<sk[b];});std::vector<Vec> ref_x(m);std::vector<unsigned> ref_k(m);for(unsigned i=0;i<m;++i){ref_x[i]=sx[order[i]];ref_k[i]=sk[order[i]];}
        auto sorted_ref=match_scalar(ref_x,ref_k);std::vector<Match> expected_scatter(m);unsigned grouped_missed=0;for(unsigned i=0;i<m;++i){expected_scatter[order[i]]=sorted_ref[i];grouped_missed+=all_ref[order[i]].count-1-(__builtin_popcount(sorted_ref[i].peers)-1);}
        regroup();finish();auto actual_scatter=scattered.read();auto device_order=sorted_order.read();for(unsigned i=0;i<m;++i){if(device_order[i]!=order[i]||source[device_order[i]]!=source[order[i]]||actual_scatter[i].peers!=expected_scatter[i].peers||actual_scatter[i].leader!=expected_scatter[i].leader)throw std::runtime_error("regroup source mapping mismatch");eq(actual_scatter[i].mean,expected_scatter[i].mean);}
        if(m>32&&(__builtin_popcount(actual_scatter[0].peers)<2))throw std::runtime_error("regroup missed controlled boundary pair");
        // Reuse grouping at the next representation level while source indices remain the scatter map.
        Buffer<Match> higher_sorted_matches(m),higher_scattered(m);Buffer<Vec> higher_sorted(m);Buffer<unsigned> higher_keys(m),higher_sorted_keys(m),higher_sorted_order(m);
        // The higher representation is the first-level mean for each member.
        auto level1=actual_scatter;std::vector<Vec> means(m);for(unsigned i=0;i<m;++i)means[i]=level1[i].mean;Buffer<Vec> dmeans(means);
        auto higher_group_real=[&]{keys_from_means<<<blocks(m),128>>>(scattered.p,higher_keys.p,m);std::size_t bytes=temp_bytes;ck(cub::DeviceRadixSort::SortPairs(temp.p,bytes,higher_keys.p,higher_sorted_keys.p,dorder.p,higher_sorted_order.p,m));gather_candidates<<<blocks(m),128>>>(dmeans.p,higher_keys.p,higher_sorted_order.p,higher_sorted.p,higher_sorted_keys.p,m);rendezvous<<<blocks(m),128>>>(higher_sorted.p,higher_sorted_keys.p,higher_sorted_matches.p,m);scatter_matches<<<blocks(m),128>>>(higher_sorted_matches.p,higher_sorted_order.p,higher_scattered.p,m);};
        higher_group_real();finish();auto high_actual=higher_scattered.read();std::vector<unsigned> high_keys(m),hi_order(m);for(unsigned i=0;i<m;++i){high_keys[i]=route_key(means[i]);hi_order[i]=i;}std::stable_sort(hi_order.begin(),hi_order.end(),[&](unsigned a,unsigned b){return high_keys[a]<high_keys[b];});std::vector<Vec> hi_values(m);std::vector<unsigned> hi_keys(m);for(unsigned i=0;i<m;++i){hi_values[i]=means[hi_order[i]];hi_keys[i]=high_keys[hi_order[i]];}auto hi_sorted_ref=match_scalar(hi_values,hi_keys);std::vector<Match> high_ref(m);for(unsigned i=0;i<m;++i)high_ref[hi_order[i]]=hi_sorted_ref[i];for(unsigned i=0;i<m;++i){if(high_actual[i].peers!=high_ref[i].peers||high_actual[i].leader!=high_ref[i].leader)throw std::runtime_error("higher-level source map mismatch");eq(high_actual[i].mean,high_ref[i].mean);}
        std::cout<<"{\"case\":\"rendezvous_supertile\",\"candidates\":"<<m<<",\"packet_fixture\":\"upper_bound_distant_sources\",\"source_span\":"<<(m?source.back()-source.front():0)<<",\"scratch_bytes\":"<<temp_bytes<<",\"same_key_partners\":"<<total_partners<<",\"no_regroup_missed\":"<<local_missed<<",\"regroup_missed\":"<<grouped_missed<<",\"higher_level_reused\":true}\n";
        measured("rendezvous","supertile_no_regroup",m,iters,direct);measured("rendezvous","supertile_all_pairs",m,iters,allpairs);measured("rendezvous","cub_sort_gather_match_scatter",m,iters,regroup);measured("rendezvous","higher_level_cub_regroup",m,iters,higher_group_real);
    }
}
int main(int argc,char** argv){
    try{
        std::string name=argc>1?argv[1]:"all";unsigned n=argc>2?bounded_number(argv[2],1u<<20):1025u;unsigned iters=argc>3?bounded_number(argv[3],10000):0u;
        if(n>(1u<<20)||iters>10000)throw std::invalid_argument("bounded prototype limits: n<=1048576, iterations<=10000");
        int count=0;auto status=cudaGetDeviceCount(&count);if(status!=cudaSuccess||!count){std::cerr<<"CUDA device unavailable\n";return 77;}
        int dev;cudaDeviceProp prop;ck(cudaGetDevice(&dev));ck(cudaGetDeviceProperties(&prop,dev));if(prop.major<7)throw std::runtime_error("requires sm_70+");
        std::cout<<"{\"device\":\""<<prop.name<<"\",\"sm\":"<<prop.major*10+prop.minor<<",\"cudart\":"<<CUDART_VERSION<<",\"untrained_fixture_weights\":true}\n";
        bool known=false;
        if(name=="all"||name=="bitlift"){test_bitlift(n,iters);known=true;}
        if(name=="all"||name=="scan"){test_scan(n,iters);known=true;}
        if(name=="all"||name=="rethread"){test_rethread(n,iters);known=true;}
        if(name=="all"||name=="rendezvous"){test_rendezvous(n,iters);known=true;}
        if(!known)throw std::invalid_argument("unknown case");
        std::cout<<"{\"phase\":\"cuda_correctness\",\"case\":\""<<name<<"\",\"n\":"<<n<<",\"passed\":true}\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
