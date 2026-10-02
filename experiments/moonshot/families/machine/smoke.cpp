#include "reference.hpp"
#include <iostream>
using namespace bp_moon;using namespace bp_moon::machine;
void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
int main(){try{
    std::string text;for(unsigned i=0;i<530;++i)text+="ACGTN"[i%5];Input in{PackedFixture(text),{11,7,900,3,Strand::forward,text.size()}};
    Surface bilinear{4,4,{}};for(unsigned y=0;y<4;++y)for(unsigned x=0;x<4;++x)bilinear.table.push_back(float(x+2*y+x*y));
    require(std::abs(response(bilinear,.25f,1.5f,true)-3.625f)<1e-6f,"bilinear interpolation");
    require(response(bilinear,-1,-1,true)==0&&response(bilinear,8,8,true)==18,"clamp convention");
    Surface jump{4,4,{}};for(unsigned y=0;y<4;++y)for(unsigned x=0;x<4;++x)jump.table.push_back(x<2?0.f:1.f);
    require(response(jump,1.5,1,true)==.5f&&response(jump,1.5,1,false)==1,"sharp response distinction");
    auto f=features(in,0);std::array<std::int8_t,4> weights{2,-1,3,-2};int dot=scalar_dot(f,weights);require(dot==5&&f.scale==.25f&&f.zero_point==0,"sequence routing score");
    std::array<float,32> lanes{};for(unsigned i=0;i<32;++i)lanes[i]=float(i)-10;auto mix=butterfly_reference(lanes),back=butterfly_reference(mix);for(unsigned i=0;i<32;++i)require(back[i]/32==lanes[i],"butterfly invertibility");
    auto permuted=lanes;std::swap(permuted[0],permuted[1]);require(butterfly_reference(permuted)!=mix,"order sensitivity");
    auto search=synthesize();require(search.survivors.size()==1&&search.lookup_survivors==1&&search.survivors[0]==std::make_pair(u32(0x0a0a),u32(0x00cc)),"bounded network search");
    for(u32 x=0;x<65536;++x)require(transpose_lookup(x)==transpose_reference(x)&&transpose_network(transpose_network(x))==x,"entire sixteen-bit permutation domain");
    require(fused(in)==staged(in)&&fused(in)==materialized(in),"fixed GC operator residency variants");
    Input bad{PackedFixture("NN"),{12,7,0,3,Strand::forward,2}};require(fused(bad)==std::vector<unsigned>{0},"invalid payload cannot count as GC");
    auto xy=coordinates(in,0);require(xy.first>=0&&xy.first<=3&&xy.second>=0&&xy.second<=3,"sequence coordinates");
    std::cout<<"E45 bilinear/clamp/discontinuity passed; E46 dot="<<dot<<" butterfly_roundtrip=32 lanes\n";
    std::cout<<"E47 candidates="<<search.tested+search.lookup_tested<<" survivors="<<search.survivors.size()+search.lookup_survivors<<" search_checks="<<search.exhaustive_checks<<" lookup_domain=65536\n";
    std::cout<<"E48 words="<<in.sequence.words.size()<<" variants=3 identical, invalid excluded\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
