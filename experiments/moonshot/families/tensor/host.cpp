#include <bp_moon/tensor.hpp>
#include <iostream>
using namespace bp_moon;
using namespace bp_moon::tensor;
void check_at(bool condition,unsigned line){if(!condition)throw std::runtime_error("sequence tensor fixture mismatch at line "+std::to_string(line));}
#define check(condition) check_at((condition),__LINE__)
Window window(u64 id,std::string sequence,u64 origin,Strand strand=Strand::forward){auto length=sequence.size();return {id,PackedFixture(sequence),{77,3,origin,9,strand,length}};}
int main(){
 auto a=window(10,"GGCT",100),b=window(20,"AAAT",10000),unknown=window(30,"NAC",20000);
 auto cohort=pack_windows({a,b,unknown});check(cohort.features[2]==2&&cohort.features[32]==1&&cohort.features[33]==1&&cohort.grounding[2].valid[0]==0&&cohort.grounding[2].exact=="NAC");
 numerical::Matrix16 weights{};weights[0]=1;weights[16]=1;weights[32]=2;weights[48]=2;weights[1]=1;weights[17]=-1;weights[33]=-1;weights[49]=1;
 auto latent=promote(cohort,weights,2);check(latent.latent.values[0]==7&&latent.latent.values[1]==-2&&latent.latent.ids[1]==20&&latent.grounding[1].coordinates.front()==10000&&latent.grounding[1].source.version==9);
 auto relations=nominate(cohort,0,1);check(relations.required==1&&!relations.overflow&&relations.pairs[0].from.id==10&&relations.pairs[0].to.id==20&&relations.pairs[0].score==7&&relations.pairs[0].to.coordinates.back()==10003);
 auto reverse=pack_windows({window(40,"ACG",500,Strand::reverse)});check(reverse.grounding[0].coordinates==std::vector<u64>({502,501,500}));
 auto left=window(50,"A",700),right=window(60,"CG",701);auto finite=concatenate_response(left,right);for(unsigned initial=0;initial<16;++initial){unsigned scalar=initial;for(char base:std::string("ACG"))scalar=(scalar*4+base_code(base))&15;for(unsigned output=0;output<16;++output)check(finite.relation[initial][output]==(output==scalar));}
 auto rr=concatenate_response(window(51,"A",704,Strand::reverse),window(61,"CG",702,Strand::reverse));check(rr.left.coordinates.back()==704&&rr.right.coordinates.front()==703);
 auto table=sample_region(a,{-1,0,1});auto query=query_region(table,.5f);check(query.error>.05f&&query.error<.07f&&query.revisit.exact=="GGCT"&&query.revisit.coordinates[0]==100);check(query_region(table,0).error==0);
 unsigned rejected=0;try{chunk_relation(unknown.sequence);}catch(const std::invalid_argument&){++rejected;}try{sample_region(unknown,{-1,1});}catch(const std::invalid_argument&){++rejected;}try{concatenate_response(left,window(60,"CG",702));}catch(const std::invalid_argument&){++rejected;}try{pack_windows({a,a});}catch(const std::invalid_argument&){++rejected;}check(rejected==4);
 std::cout<<"E21 source-grounded counts/promote passed; E22 distant directed pair+invalid mask passed; E23 adjacent chunk scalar oracle passed; E24 GC-conditioned query error="<<query.error<<" exact revisit passed\n";
}
