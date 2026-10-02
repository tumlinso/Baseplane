#include "hierarchy.hpp"
#include <iostream>
#ifdef BP_HIERARCHY_LEARNING
#include "learning.hpp"
#endif
using namespace bp_moon;
using namespace bp_moon::hierarchy;
void require(bool ok,const char* label){if(!ok)throw std::runtime_error(label);}
template<class F> void rejects(F f){bool yes=false;try{f();}catch(const std::exception&){yes=true;}require(yes,"expected rejection");}
std::size_t positive(const std::vector<double>& x){return std::count_if(x.begin(),x.end(),[](double v){return v>.5;});}
#ifdef BP_HIERARCHY_LEARNING
std::vector<double> features(const PackedFixture& s){
    std::vector<double> x;for(std::size_t p=1;p<s.original.size();++p){
        double l=gc(s.original[p-1]),r=gc(s.original[p]);
        x.insert(x.end(),{l,r,l!=r?1.:0.,s.is_valid(p-1)&&s.is_valid(p)?1.:0.});
    }return x;
}
std::vector<Span> learned_chunks(const PackedFixture& s,const double* weights){
    auto x=features(s);std::vector<std::size_t> cuts;std::size_t previous=0;
    for(std::size_t p=1;p<s.original.size();++p){
        if(p-previous>=16||(p-previous>=2&&ce_moon::learning::predict_logistic(x.data()+(p-1)*4,4,weights)>.5)){
            cuts.push_back(p);previous=p;
        }
    }return chunks(s.original.size(),cuts);
}
#endif
int main(){
 try {
    PackedFixture s("GAGAGANAGAGAG");SourceMap m{7,9,100,3,Strand::reverse,s.original.size()};
    Reservoir reservoir(s,m);auto full=reservoir.expand(true),coarse=reservoir.expand(false);
    require(full==reservoir.replay(s,m),"E13 roundtrip");require(positive(full)!=positive(coarse),"E13 omission changes query");
    auto coords=support(s,m,{0,s.original.size()});require(coords.size()==12&&coords.front()==112&&coords.back()==100,"valid reverse source support");
    auto stale=m;++stale.version;rejects([&]{reservoir.replay(s,stale);});
    auto overflow=m;overflow.origin=std::numeric_limits<u64>::max();rejects([&]{Reservoir bad(s,overflow);});
    PackedFixture odd("CAN");SourceMap om{7,9,0,3,Strand::forward,3};Reservoir tail(odd,om);
    require(tail.expand(true)==tail.replay(odd,om),"E13 tail");
    u64 emitted[2];auto counts=emit_valid_positions(s,~u32{0},0,emitted,2,m);
    require(counts.produced==12&&counts.stored==2&&counts.dropped==10,"capacity accounting");
    std::cout<<"E13 full_positive="<<positive(full)<<" coarse_positive="<<positive(coarse)<<" residual_bytes="<<reservoir.residual_bytes()<<" source_replayed="<<full.size()<<"\n";

    PackedFixture zeros(std::string(1024,'A')),straddle(std::string(513,'G')+std::string(511,'A'));
    auto easy=precision_query(zeros,{0,1024},.75),hard=precision_query(straddle,{0,1024},.5);
    require(!easy.above&&hard.above&&easy.planes<hard.planes,"E15 ambiguous refinement");
    require(hard.lower<=513/1024.&&hard.upper>=513/1024.,"E15 interval contains truth");
    PackedFixture invalid("GNNN");auto masked=precision_query(invalid,{0,4},1/1024.);
    require(!masked.above,"E15 invalid exclusion and strict threshold");
    rejects([&]{precision_query(zeros,{0,1025},.5);});
    std::cout<<"E15 easy_planes="<<easy.planes<<" ambiguous_planes="<<hard.planes<<" lower="<<hard.lower<<" upper="<<hard.upper<<"\n";

    PackedFixture before("ACGTGCAATGCCATATGCGTACCGTTAGCGATACGTGCAATGCCATATGCGTACCGTTAGCGAT");
    auto edited=before.original;edited.insert(edited.begin()+3,'G');PackedFixture after(edited);
    auto a=content_chunks(before,3,10),b=content_chunks(after,3,10),alternative=content_chunks(before,3,10,1);
    for(auto spans:{a,b})for(std::size_t i=0;i<spans.size();++i){auto len=spans[i].end-spans[i].begin;require(len<=10&&(i+1==spans.size()||len>=3),"E16 bounded chunks");}
    require(reused_chunks(before,a,after,b)>0,"E16 insertion reuse exact bytes");
    PackedFixture repeated(std::string(75,'A'));auto repeat_chunks=content_chunks(repeated,3,10);require(repeat_chunks.size()>=8,"E16 repetitive cap");
    auto seam=a.front().end;std::string motif=before.original.substr(seam-1,3);
    auto answers=motif_query(before,motif,a,alternative);
    require(std::find(answers.positions.begin(),answers.positions.end(),seam-1)!=answers.positions.end(),"E16 crossing seam preserved");
    auto spanning=motif_query(before,before.original,a,alternative);require(spanning.positions==std::vector<std::size_t>{0}&&spanning.replayed>0,"E16 replay when both seams cut");
    PackedFixture broken("ANCG");require(motif_query(broken,"AA",chunks(4,{}),chunks(4,{})).positions.empty(),"E16 invalid payload rejected");
    inverse_map(before.original.size(),a);rejects([&]{content_chunks(before,0,10);});
    std::cout<<"E16 chunks_before="<<a.size()<<" chunks_after="<<b.size()<<" exact_reused="<<reused_chunks(before,a,after,b)<<" alternative_windows="<<answers.alternative<<" replay_windows="<<answers.replayed<<"\n";
#ifdef BP_HIERARCHY_LEARNING
    std::string training;for(int i=0;i<24;++i)training+="AAAAAAAAGGGGGGGG";
    PackedFixture train(training);auto x=features(train);std::vector<double> targets;
    for(std::size_t p=1;p<train.original.size();++p)targets.push_back(gc(train.original[p-1])!=gc(train.original[p]));
    double weights[5]{};ce_moon::learning::FitOptions options;options.budget_weight=0.5;options.positive_budget=0.15;
    auto fit=ce_moon::learning::fit_logistic(x.data(),targets.data(),targets.size(),4,weights,options);(void)fit;
    auto learned=learned_chunks(before,weights),learned_after=learned_chunks(after,weights);
    inverse_map(before.original.size(),learned);inverse_map(after.original.size(),learned_after);
    auto transition=ce_moon::learning::predict_logistic(std::array<double,4>{0,1,1,1}.data(),4,weights);
    auto homogeneous=ce_moon::learning::predict_logistic(std::array<double,4>{0,0,0,1}.data(),4,weights);
    require(transition>homogeneous,"E14 fitted contextual boundary distinction");
    auto mapping=inverse_map(s.original.size(),learned_chunks(s,weights));
    for(std::size_t p=0;p<s.original.size();++p){require(mapping[p]<s.original.size(),"E14 inverse map");require(m.coordinate(p)==112-p,"E14 exact reverse coordinate");}
    std::vector<std::size_t> fixed_cuts;for(std::size_t p=8;p<before.original.size();p+=8)fixed_cuts.push_back(p);
    std::vector<std::size_t> surprise_cuts;for(std::size_t p=1;p<before.original.size();++p)if(gc(before.original[p-1])!=gc(before.original[p]))surprise_cuts.push_back(p);
    auto fixed=chunks(before.original.size(),fixed_cuts),surprise=chunks(before.original.size(),surprise_cuts);
    fixed_cuts.clear();for(std::size_t p=8;p<after.original.size();p+=8)fixed_cuts.push_back(p);
    surprise_cuts.clear();for(std::size_t p=1;p<after.original.size();++p)if(gc(after.original[p-1])!=gc(after.original[p]))surprise_cuts.push_back(p);
    auto fixed_after=chunks(after.original.size(),fixed_cuts),surprise_after=chunks(after.original.size(),surprise_cuts);
    std::string reconstructed;for(auto span:learned)reconstructed+=before.original.substr(span.begin,span.end-span.begin);
    require(reconstructed==before.original,"E14 dechunk exact bases");
    std::cout<<"E14 transition_probability="<<transition<<" homogeneous_probability="<<homogeneous<<" learned_chunks="<<learned.size()<<" inserted_chunks="<<learned_after.size()<<" fixed_chunks="<<fixed.size()<<" surprise_chunks="<<surprise.size()<<" learned_reused="<<reused_chunks(before,learned,after,learned_after)<<" fixed_reused="<<reused_chunks(before,fixed,after,fixed_after)<<" surprise_reused="<<reused_chunks(before,surprise,after,surprise_after)<<" inverse_positions="<<mapping.size()<<"\n";
#else
    std::cout<<"E14 unavailable: supply CE_MOON_LEARNING_DIR for provider-backed fit\n";
#endif
    std::cout<<"hierarchy semantic checks passed; CPU fixture only\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
