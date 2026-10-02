#include <compiler.hpp>
#include <logic.hpp>
#include <rendezvous.hpp>
#include <iostream>
using namespace bp_moon;
void check(bool b,const char* message){if(!b)throw std::runtime_error(message);}
int main(){try{
 namespace ce=ce_moon::learning;
 auto teacher=ce::fit_truth_table(compiler::training_labels(),200,0.5,17);
 auto hardened=ce::harden(teacher,23);auto banks=compiler::train_banks(29);
 std::vector<discovery::Sequence> corpus;
 std::vector<compiler::Hit> routed_hits;
 unsigned teacher_decisions=0;
 for(const auto& text:{std::string("AAGT"),std::string("CAGT"),std::string("CCGT"),std::string("ANGT")}){
  SourceMap source{u64(100+corpus.size()),7,u64(1000+100*corpus.size()),1,Strand::forward,text.size()};
  PackedFixture sequence(text);auto features=compiler::planes(sequence,0);
  logic::Domain domain{source.source_id,source.contig,source.version,0};
  logic::Mask a{domain,features.gc,features.valid},b{domain,features.previous_a,features.valid},c{domain,features.next_t,features.valid};
  auto mask=logic::Circuit{hardened.immediate,0xF0,hardened.circuit_version}.apply(a,b,c);
  check(mask.bits==compiler::hard_decisions(features,hardened),"teacher-generated immediate executes in logic circuit");
  for(unsigned lane=0;lane<text.size();++lane){auto f=compiler::features(sequence,lane);if(!f.valid)continue;
   check(bool(mask.bits&(u32{1}<<lane))==(teacher.predict(f.row)>=0.5),"floating teacher and hardened sequence decisions");teacher_decisions+=bool(mask.bits&(u32{1}<<lane));}
  const u64 query_domain=source.source_id;auto rewritten=compiler::compile_question(query_domain);
  auto reduced_mask=compiler::rewritten_mask(rewritten,features,query_domain);
  std::vector<compiler::Hit> compact(text.size());auto emitted=compiler::hardened_query(sequence,source,hardened,{17,23,5,0,0},compact.data(),compact.size());compact.resize(emitted.stored);
  std::vector<compiler::Hit> bank_hits(text.size());auto bank=compiler::bank_query(sequence,source,banks,2,31,bank_hits.data(),bank_hits.size());bank_hits.resize(bank.counts.stored);
  check(bank.selection.index==1,"context-conditioned GT bank");
  std::vector<compiler::Hit> alternate_hits(text.size());auto alternate=compiler::bank_query(sequence,source,banks,-2,32,alternate_hits.data(),alternate_hits.size());
  check(alternate.selection.index==0&&alternate.counts.produced==0,"changed context selects distinct exact predicate bank");u32 selected=0;
  for(const auto& hit:bank_hits)selected|=u32{1}<<unsigned(hit.anchor-source.origin);
  logic::Mask request{domain,selected&reduced_mask,features.valid};
  auto routed=logic::route(mask,request,compact);
  for(const auto& hit:routed){check(hit.provenance.weights==17&&hit.provenance.circuit==23&&hit.source_id==source.source_id,"learned circuit provenance survives route");routed_hits.push_back(hit);}
  // Only source windows admitted by actual hard-circuit/bank/rewrite results
  // enter the approximate nomination corpus. No handcrafted candidate keys.
  if(!routed.empty())corpus.push_back({std::move(sequence),source});
 }
 check(corpus.size()==3&&routed_hits.size()==3&&teacher_decisions==3,"invalid-context source not admitted");
 discovery::Costs extraction_cost;auto objects=discovery::extract(corpus,4,extraction_cost);
 check(objects.size()==3,"learned outputs build real candidate objects");
 discovery::Costs one_cost,two_cost;auto single=discovery::nominate(objects,1,1,one_cost);auto multi=discovery::nominate(objects,2,1,two_cost);
 check(single.missed==2&&multi.missed==0,"second independent half probe recovers fixture misses");
 check(multi.candidates.size()==3&&multi.verified.size()==2,"exact Hamming verification rejects false candidate");
 std::size_t motif_verified=0;
 for(auto pair:multi.verified){const auto& left=objects.at(pair.first);const auto& right=objects.at(pair.second);
  check(discovery::distance(left.exact,right.exact)<=1,"verified candidate obeys query tolerance");
  // Separate exact source replay verifies the bank's two-base witness, retaining
  // both participants' source/version/coordinates rather than relabeling near
  // whole-window agreement as exact identity.
  for(const auto* object:{&left,&right}){const auto& source=corpus.at(object->id);const auto p=object->local+2;
   check(source.sequence.is_valid(p)&&source.sequence.is_valid(p+1)&&source.sequence.original.substr(p,2)=="GT","exact motif witness after nomination");
   check(object->source.version==1&&object->first==source.source.coordinate(object->local)&&object->last==source.source.coordinate(object->local+3),"pair provenance retained");}
  ++motif_verified;
 }
 check(motif_verified==2&&two_cost.verifications==3,"actual nominees drive exact replay");
 std::cout<<"{\"composition\":\"C05\",\"status\":\"pass\",\"learned_admitted_objects\":3,\"nominees\":3,\"exact_verified_pairs\":2,\"one_probe_missed\":2,\"two_probe_missed\":0,\"gpu_executed\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
