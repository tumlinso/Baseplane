#include "logic.hpp"
#include <iostream>
#include <random>
using namespace bp_moon;
using namespace bp_moon::logic;
void check(bool b,const char* message){if(!b)throw std::runtime_error(message);}
template<class F> void rejects(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught,"expected rejection");}
int main(){try{
 for(unsigned lut=0;lut<256;++lut)for(unsigned row=0;row<8;++row)
  check((lut3(row&4?~u32{0}:0,row&2?~u32{0}:0,row&1?~u32{0}:0,lut)&1)==((lut>>row)&1),"LUT truth row");
 PackedFixture seq(std::string(31,'T')+"ACGNTACG");Domain domain{9,2,4,0};
 auto p=seq.planes(0);Mask a{domain,p.lo,p.valid},b{domain,p.hi,p.valid},c{domain,~u32{0},p.valid};
 auto circuit=Circuit{}.apply(a,b,c);
 for(unsigned i=0;i<32;++i)check(((circuit.bits>>i)&1)==((((a.bits>>i)&1)^((b.bits>>i)&1)^1)&((b.bits>>i)&1)),"circuit scalar");
 auto tail=seq.planes(1);a={Domain{9,2,4,1},tail.lo,tail.valid};b={a.domain,tail.hi,tail.valid};c={a.domain,~u32{0},tail.valid};
 check(!(Circuit{}.apply(a,b,c).bits&~tail.valid),"circuit tail invalid");
 Evidence counts(0x7fffffff);for(unsigned i=0;i<15;++i)counts.add(~u32{0});
 check(counts.wrap.at(0)==15&&!counts.wrap.overflow&&counts.at_least(15)==0x7fffffff,"fifteen supports");
 counts.add(~u32{0});check(counts.wrap.at(0)==0&&counts.wrap.overflow==0x7fffffff,"explicit wrap overflow");
 check(counts.saturated.at(0)==15&&counts.saturated.overflow==0x7fffffff,"saturation distinct");
 check(counts.at_least(1)==0&&counts.at_least(15,true)==0x7fffffff,"threshold semantics");
 std::mt19937 rng(2020);Evidence random(~u32{0});std::array<unsigned,32> scalar{};
 for(unsigned j=0;j<11;++j){u32 v=rng();random.add(v);for(unsigned l=0;l<32;++l)scalar[l]+=(v>>l)&1;}
 for(unsigned t=0;t<16;++t)for(unsigned l=0;l<32;++l)check(((random.at_least(t)>>l)&1)==unsigned(scalar[l]>=t),"bitplane scalar threshold");
 SourceMap map{9,2,100,4,Strand::forward,seq.original.size()};
 auto hits=grammar(seq,map,"ACG",Duplicates::both_strands);
 check(hits.size()==2&&hits[0].coordinate==131&&hits[1].coordinate==136,"seam ownership");
 PackedFixture reverse(std::string(31,'A')+"CGTN");map.length=reverse.original.size();
 hits=grammar(reverse,map,"ACG",Duplicates::both_strands);
 check(hits.size()==2&&hits[0].coordinate==130&&hits[1].coordinate==133&&hits[1].strand==Strand::reverse,"reverse complement coordinates");
 PackedFixture palindrome("ATNAT");map.length=5;
 check(grammar(palindrome,map,"AT",Duplicates::both_strands).size()==4,"palindrome both");
 check(grammar(palindrome,map,"AT",Duplicates::canonical_anchor).size()==2,"palindrome canonical");
 PackedFixture invalid("ANG");map.length=3;check(grammar(invalid,map,"ACG",Duplicates::both_strands).empty(),"invalid motif payload");
 for(unsigned trial=0;trial<102;++trial){u32 mask=trial==0?0:trial==1?~u32{0}:rng();
  for(unsigned rank=0;rank<popcount(mask);++rank)check(rank_before(mask,select_bit(mask,rank))==rank,"rank/select roundtrip");}
 Mask parent{domain,0x80000022,~u32{0}},query{domain,0x80000002,~u32{0}};
 check(route(parent,query,std::vector<unsigned>{11,55,3131})==std::vector<unsigned>({11,3131}),"compact query descriptors");
 check(compose_support(parent.bits,5)==query.bits,"local parent composition");
 check(compose_support(0,0)==0&&compose_support(~u32{0},~u32{0})==~u32{0},"empty/full composition");
 rejects([&]{auto other=query;++other.domain.version;intersect(parent,other);});
 rejects([&]{compose_support(1,2);});rejects([&]{select_bit(0,0);});
 std::cout<<"{\"status\":\"pass\",\"cards\":4,\"mechanisms\":4,\"gpu_executed\":false}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
