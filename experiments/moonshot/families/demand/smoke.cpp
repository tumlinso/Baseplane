#include "demand.hpp"
#include <iostream>
using namespace bp_moon;using namespace bp_moon::demand;
void check(bool b,const char* m){if(!b)throw std::runtime_error(m);}
template<class F>void rejects(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught,"expected rejection");}
int main(){try{
 PackedFixture seq("ACGNTAC");SourceMap map{12,4,100,7,Strand::forward,seq.original.size()};
 const auto bound=certificate(seq,map,0,3);
 check(prunes(bound,Question::max_at_least,3,bound.domain),"exact max bound");
 check(!prunes(bound,Question::gc_count,3,bound.domain),"query specific certificate");
 rejects([&]{auto foreign=bound.domain;++foreign.source;prunes(bound,Question::max_at_least,3,foreign);});
 rejects([&]{auto stale=bound.domain;++stale.version;prunes(bound,Question::max_at_least,3,stale);});
 auto max=pull(seq,map,Question::max_at_least,3,32,100,7);
 check(max.selected.size()==1&&max.selected[0].begin==104&&max.deferred.empty()&&max.pruned>0,"pull maximum fixture");
 auto gc=pull(seq,map,Question::gc_count,3,32,100,7);
 check(gc.selected.size()==3&&gc.selected[0].begin==101&&gc.selected[1].begin==102&&gc.selected[2].begin==106,"GC different question");
 auto limited=pull(seq,map,Question::gc_count,0,1,2,7);
 check(!limited.deferred.empty()&&limited.examined==2,"bounded query replay");
 check(pull(seq,map,Question::gc_count,0,0,100,7).deferred.size()==2,"zero queue explicit deferral");
 rejects([&]{pull(seq,map,Question::gc_count,0,32,100,8);});
 std::vector<Carrier> carriers;for(std::size_t p=0;p<map.length;++p)carriers.push_back({p,support(map,p,p+1),Opcode::fetch});
 auto waves=execute(seq,map,carriers,7,3);
 check(waves.pending.empty()&&waves.selected.size()==3&&waves.produced==7&&waves.stored==7,"opcode cohort waves");
 auto one=execute(seq,map,carriers,7,1);check(one.pending.size()==7,"wave budget pending");
 auto short_queue=execute(seq,map,carriers,2,3);check(short_queue.deferred.size()==5&&short_queue.selected.size()==1,"capacity exhaustion retains work");
 check(execute(seq,map,{},0,3).pending.empty(),"empty wave");
 rejects([&]{auto bad=carriers;bad[0].opcode=Opcode::test_gc;bad[0].valid=true;bad[0].payload=2;execute(seq,map,bad,7,3);});
 std::vector<Bid> bids;for(unsigned p=0;p<5;++p)bids.push_back({support(map,p,p+1),p==4?0u:100u,1});
 std::size_t cursor=0;bool quiet=false;std::set<std::size_t> visited;
 for(unsigned round=0;round<5;++round){auto auctioned=auction(bids,1,cursor);check(auctioned.spent==1&&auctioned.selected.size()==1,"fixed cost budget");visited.insert(auctioned.selected[0]);if(auctioned.selected[0]==4)quiet=true;}
 check(quiet&&visited.size()==5,"quiet sequence finite revisit");
 check(bids[4].debt==0&&bids[0].debt==4,"debt visible");
 rejects([&]{auto impossible=bids;impossible[0].cost=2;auction(impossible,1,cursor);});
 std::vector<Cell> cells;for(unsigned p=0;p<3;++p)cells.push_back({support(map,p,p+1)});
 jacobi(cells,3);jacobi(cells,3);check(cells[0].promoted&&cells[2].promoted,"stable promotion");
 wake(cells,{2,999,8,0});check(cells[2].active&&!cells[2].promoted&&cells[2].context_version==1,"remote context wake");
 jacobi(cells,3);check(cells[1].active,"neighbor frontier wake");
 check(cells[1].context==8&&cells[0].context==0,"Jacobi uses previous wave");
 jacobi(cells,3);check(cells[0].context==8&&cells[0].message_sources.count(999),"long range lineage propagates");
 check(cells[0].source.begin==100&&cells[0].source.version==7,"fine source survives");
 rejects([&]{wake(cells,{2,999,16,0});});
 for(auto& cell:cells)cell.active=true;
 auto bounded_wave=jacobi(cells,1);check(bounded_wave.updated==1&&bounded_wave.deferred==2&&cells[2].active,"frontier deferral persistent");
 std::cout<<"{\"status\":\"pass\",\"cards\":4,\"host_mechanisms\":4,\"gpu_executed\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
