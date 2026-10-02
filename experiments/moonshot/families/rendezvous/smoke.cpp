#include "rendezvous.hpp"
#include <iostream>
using namespace bp_moon;using namespace bp_moon::discovery;
void require(bool x,const char* why){if(!x)throw std::runtime_error(why);}
Sequence seq(std::string text,u64 id=1,u64 origin=100,Strand strand=Strand::forward){auto n=text.size();return {PackedFixture(std::move(text)),{id,7,origin,9,strand,n}};}
int main(){
    try {
        Costs c;auto objects=extract({seq(std::string(99,'A')),seq("NNacgtN",2,400,Strand::reverse)},3,c);
        require(objects.size()==99,"whole input/tail/validity");
        require(objects[97].exact=="ACG"&&objects[97].first==404&&objects[97].last==402&&objects[97].source.version==9,"reverse provenance");
        Directory d(objects);auto pairs=tiled_pairs(d,37,32);
        require(d.groups[0].ids.size()==97&&pairs.counts.produced==97*96&&pairs.counts.stored==37&&pairs.counts.dropped==97*96-37,"complete group/bounded output");
        require(tiled_pairs(d,0).counts.dropped==97*96,"zero capacity");
        Costs hc;PostingHash h(d,1,1,hc);
        require(h.overflow==2&&hc.collisions>0,"forced overflow");
        for(const auto& g:d.groups){const auto* found=h.find(g.key,hc);require(found&&found->ids==g.ids,"hash full membership");}
        require(!h.find("CCC",hc),"absent key");
        const auto gpu_rows=packed_rows(h);
        require(gpu_rows.size()==h.rows.size()&&gpu_rows.front().slots[0].group==0&&gpu_rows.front().slots[0].key==0,"zero key is not empty sentinel");
        require(packed_key("A",0).width!=packed_key("AA",0).width,"packed length identity");
        require(gpu_rows.back().next==gpu::absent,"overflow chain packing");
        Costs nc;auto neighbors=extract({seq("AAAA",3),seq("CAAA",4),seq("CAAT",5),seq("AAAA",6)},4,nc);
        auto first=nominate(neighbors,1,1,nc),second=nominate(neighbors,2,1,nc);
        require(!first.candidates.count({0,1})&&second.verified.count({0,1}),"second probe recovery");
        require(second.candidates.count({0,3})==1,"deduplication");
        auto misses=nominate(neighbors,2,2,nc);require(misses.missed>0,"honest approximation miss");
        auto sources=std::vector<Sequence>{seq("ACGNACGACG",8,100),seq("acg",9,500,Strand::reverse)};
        auto requests=std::vector<Request>{{"ACG",10,8,7,100,103,Strand::forward},{"ACG",11,8,7,104,110,Strand::forward},{"ACG",12,8,7,100,110,Strand::forward},{"ACG",13,8,7,101,104,Strand::forward},{"ACG",14,9,7,500,503,Strand::reverse}};
        auto answers=subscribe(sources,requests,20);
        require(answers.records.size()==7&&answers.evaluations==9,"shared evaluation/routing");
        require(answers.records.front().destination==10&&answers.records.front().version==9,"answer source provenance");
        require(std::any_of(answers.support.begin(),answers.support.end(),[](auto s){return s.destination==13;}),"negative inspection support");
        auto limited=subscribe(sources,requests,2);require(limited.counts.produced==7&&limited.counts.stored==2&&limited.counts.dropped==5,"bounded answers");
        require(Directory({}).groups.empty()&&tiled_pairs(Directory({}),0).counts.produced==0,"empty directory");
        bool rejected=false;try{extract({seq("A")},0,c);}catch(const std::invalid_argument&){rejected=true;}require(rejected,"invalid width");
        std::cout<<"E17 objects="<<objects.size()<<" inspected="<<c.inspected<<" groups="<<d.groups.size()<<" pairs="<<pairs.counts.produced<<" stored="<<pairs.counts.stored<<" dropped="<<pairs.counts.dropped<<'\n';
        std::cout<<"E18 overflow_rows="<<h.overflow<<" probes="<<hc.probes<<" collisions="<<hc.collisions<<'\n';
        std::cout<<"E19 primary_candidates="<<first.candidates.size()<<" multiprobe_candidates="<<second.candidates.size()<<" verified="<<second.verified.size()<<" evaluation_comparisons="<<misses.evaluation_comparisons<<" adversarial_missed="<<misses.missed<<'\n';
        std::cout<<"E20 evaluations="<<answers.evaluations<<" inspected_supports="<<answers.support.size()<<" answers="<<answers.records.size()<<" bounded_dropped="<<limited.counts.dropped<<'\n';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
