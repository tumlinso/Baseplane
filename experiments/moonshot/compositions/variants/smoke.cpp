#include <bp_moon/ports.hpp>
#include <incremental.hpp>
#include <iostream>
using namespace bp_moon;
void check(bool b,const char* message){if(!b)throw std::runtime_error(message);}
int main(){try{
 // World zero is baseline, world one changes outside memo support, world two
 // changes inside it, world three inserts an invalid payload into that support.
 PackedFixture baseline("AACGTA");SourceMap source{800,6,1000,1,Strand::forward,6};
 auto worlds=ports::alternative_worlds(baseline,source,{{900,{}},{901,{{5,'C'}}},{902,{{2,'A'}}},{903,{{2,'N'}}}});
 check(worlds.support.size()==4&&worlds.numerical.states.size()==4,"E44 worlds produced");
 incremental::RepeatQuotient quotient;const incremental::QueryIdentity query{1,2,3,4};
 std::size_t reused=0,recomputed=0;std::vector<unsigned> answers;
 for(std::size_t w=0;w<worlds.support.size();++w){const auto& produced=worlds.support[w];
  // Explicit derived-world identity avoids aliasing alternate states at the same
  // source revision. The E44 support also retains original baseline identity.
  SourceMap world_base=source;world_base.source_id=produced.world_id;
  quotient.add(baseline,world_base,1,0,42);
  auto memo=incremental::Memo::count(baseline,world_base,query,{0,3});
  incremental::DirtyCone cone({{2},{2},{}},{baseline.original},{world_base});
  PackedFixture exact(produced.exact_sequence);auto current=world_base;
  std::vector<incremental::Edit> edits;std::size_t changes=0;
  for(std::size_t p=0;p<exact.original.size();++p)if(exact.original[p]!=baseline.original[p]){
   ++changes;edits.push_back({1,2,p,1,1});}
  check(changes<=1,"composition has one edit per branch");
  std::vector<unsigned> dirty;if(changes){current.version=2;dirty=cone.replace(0,1,exact.original,current);check(dirty==std::vector<unsigned>({0,1,2}),"directory and effect dependency invalidation");}
  bool reusable=memo.reusable(current,query,edits);
  // The dependency invalidation output is the actual trigger for guarded memo
  // inspection. An unaffected leaf may reuse; a dirty parent may not blindly do so.
  unsigned answer=memo.answer;
  if(!dirty.empty()&&std::find(dirty.begin(),dirty.end(),2)!=dirty.end()&&!reusable){memo=incremental::Memo::count(exact,current,query,{0,3});answer=memo.answer;++recomputed;}
  else {check(reusable,"clean memo guard");++reused;}
  check(answer==incremental::gc_count(exact,{0,3}),"world answer compared with independent execution");answers.push_back(answer);
  double direct=0;for(std::size_t p=0;p<exact.original.size();++p)if(exact.is_valid(p))direct+=ports::code(exact.original[p])+1;
  check(std::abs(worlds.numerical.states[w][worlds.query_node]-direct)<1e-12,"CE delta-world answer compared with independent execution");
  for(const auto& site:produced.valid_support)check(site.source_id==source.source_id&&site.version==source.version&&site.coordinate==source.origin+site.position&&exact.is_valid(site.position),"world keeps baseline lineage and validity");
  quotient.add(exact,current,2,worlds.numerical.states[w][worlds.query_node]-worlds.numerical.states[0][worlds.query_node],42);
  check(cone.directory.count(exact.original)==1,"world directory membership");
 }
 check(reused==2&&recomputed==2&&answers==std::vector<unsigned>({1,1,0,0}),"memo reuse and invalidation branches");
 check(quotient.contents.size()==4&&quotient.occurrences.size()==8,"baseline repeats shared, changed world content separate");
 auto stale_query=query;++stale_query.weights;
 auto memo=incremental::Memo::count(baseline,source,query,{0,3});check(!memo.reusable(source,stale_query,{}),"weights invalidate memo");
 check(worlds.query_groups.size()==4,"CE grouped query world labels retained");
 std::cout<<"{\"composition\":\"C03\",\"status\":\"pass\",\"worlds\":4,\"memo_reused\":2,\"memo_recomputed\":2,\"gpu_executed\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
