#pragma once
#include <bp_moon/source.hpp>
#include <set>

namespace bp_moon::demand {
constexpr std::size_t fixture_limit=4096;
inline void bounded(std::size_t n){if(n>fixture_limit)throw std::invalid_argument("fixture exceeds bounded domain");}
inline unsigned code(char c){switch(c){case 'A':case 'a':return 0;case 'C':case 'c':return 1;case 'G':case 'g':return 2;case 'T':case 't':return 3;}throw std::invalid_argument("invalid base");}
struct Support {u64 source,contig,begin,end,version;};
inline Support support(const SourceMap& map,std::size_t begin,std::size_t end){
 if(begin>=end||end>map.length)throw std::invalid_argument("support interval");
 const auto a=map.coordinate(begin),b=map.coordinate(end-1);
 const auto last=std::max(a,b);if(last==std::numeric_limits<u64>::max())throw std::overflow_error("half-open support");
 return {map.source_id,map.contig,std::min(a,b),last+1,map.version};
}
inline void validate(const PackedFixture& seq,const SourceMap& map){bounded(map.length);if(map.length!=seq.original.size())throw std::invalid_argument("source length");}
// E25: an exact source-derived bound is scoped to query, support and source version.
// A maximum-base-code certificate cannot answer a GC-count question.
enum class Question {max_at_least,gc_count};
struct Certificate {Support domain;unsigned max_code;bool any_valid;};
inline bool prunes(const Certificate& c,Question q,unsigned threshold,const Support& expected){
 if(c.domain.source!=expected.source||c.domain.contig!=expected.contig||c.domain.begin!=expected.begin||c.domain.end!=expected.end||c.domain.version!=expected.version)throw std::invalid_argument("stale or foreign certificate");
 return q==Question::max_at_least&&(!c.any_valid||c.max_code<threshold);
}
inline Certificate certificate(const PackedFixture& seq,const SourceMap& map,std::size_t begin,std::size_t end){
 Certificate c{support(map,begin,end),0,false};
 for(auto p=begin;p<end;++p)if(seq.is_valid(p)){c.any_valid=true;c.max_code=std::max(c.max_code,code(seq.original[p]));}
 return c;
}
struct QueryResult {std::vector<Support> selected,deferred;std::size_t examined=0,pruned=0;};
inline QueryResult pull(const PackedFixture& seq,const SourceMap& map,Question q,unsigned threshold,
                        std::size_t wave_capacity,std::size_t work_budget,u64 expected_version){
 validate(seq,map);bounded(wave_capacity);if(expected_version!=map.version)throw std::invalid_argument("query source version");
 struct Region{std::size_t begin,end,depth;};
 QueryResult out;if(!map.length)return out;
 std::vector<Region> queue{{0,map.length,0}};
 for(std::size_t cursor=0;cursor<queue.size();++cursor){auto region=queue[cursor];
  if(out.examined>=work_budget){out.deferred.push_back(support(map,region.begin,region.end));continue;}
  ++out.examined;auto bound=certificate(seq,map,region.begin,region.end);
  if(prunes(bound,q,threshold,bound.domain)){++out.pruned;continue;}
  if(region.end-region.begin==1){const auto p=region.begin;
   if(seq.is_valid(p)&&(q==Question::max_at_least?code(seq.original[p])>=threshold:code(seq.original[p])==1||code(seq.original[p])==2))out.selected.push_back(bound.domain);
   continue;
  }
  const auto middle=region.begin+(region.end-region.begin)/2;
  // Queue storage has an explicit per-wave admission limit. Deferred support is
  // returned for replay, retaining the exact ranges rather than a bounding box.
  for(auto child:{Region{region.begin,middle,region.depth+1},Region{middle,region.end,region.depth+1}}){
   if(queue.size()-cursor-1<wave_capacity)queue.push_back(child);
   else out.deferred.push_back(support(map,child.begin,child.end));
  }
 }
 return out;
}
// E26: stable opcode cohorts execute bounded waves, not a spinning GPU queue.
enum class Opcode {fetch,test_gc};
struct Carrier {std::size_t position;Support source;Opcode opcode;unsigned payload=0;bool valid=false;};
struct Waves {std::vector<Carrier> pending,deferred,selected;std::size_t produced=0,stored=0;};
inline Waves execute(const PackedFixture& seq,const SourceMap& map,const std::vector<Carrier>& initial,
                     std::size_t capacity,unsigned epochs){
 validate(seq,map);bounded(initial.size());bounded(capacity);if(epochs>64)throw std::invalid_argument("epoch budget");
 Waves out;
 for(const auto& item:initial){
  const auto expected=support(map,item.position,item.position+1);
  if(item.source.source!=expected.source||item.source.contig!=expected.contig||item.source.version!=expected.version||item.source.begin!=expected.begin||item.source.end!=expected.end)throw std::invalid_argument("carrier provenance");
  if(item.opcode!=Opcode::fetch&&item.opcode!=Opcode::test_gc)throw std::invalid_argument("opcode");
  if(item.opcode==Opcode::test_gc&&(item.valid!=seq.is_valid(item.position)||(item.valid&&item.payload!=code(seq.original[item.position]))))throw std::invalid_argument("unverified payload");
  if(out.pending.size()<capacity)out.pending.push_back(item);else out.deferred.push_back(item);
 }
 for(unsigned wave=0;wave<epochs&&!out.pending.empty();++wave){std::vector<Carrier> next;
  for(auto opcode:{Opcode::fetch,Opcode::test_gc})for(auto item:out.pending)if(item.opcode==opcode){
   if(opcode==Opcode::fetch){item.valid=seq.is_valid(item.position);if(item.valid)item.payload=code(seq.original[item.position]);item.opcode=Opcode::test_gc;
    ++out.produced;if(next.size()<capacity){next.push_back(item);++out.stored;}else out.deferred.push_back(item);
   }else if(item.valid&&(item.payload==1||item.payload==2))out.selected.push_back(item);
  }
  out.pending=std::move(next);
 }
 return out;
}
// E27: numerical values are supplied by the caller (CE learning provider seam).
struct Bid {Support source;unsigned value,cost,debt=0;};
struct Auction {std::vector<std::size_t> selected,deferred;unsigned spent=0;};
inline Auction auction(std::vector<Bid>& bids,unsigned budget,std::size_t& cursor){
 bounded(bids.size());Auction out;if(bids.empty())return out;
 for(auto& bid:bids)if(!bid.cost||bid.cost>budget)throw std::invalid_argument("unserviceable bid cost");
 std::vector<bool> served(bids.size());
 auto take=[&](std::size_t i){served[i]=true;out.selected.push_back(i);out.spent+=bids[i].cost;};
 // Reserve one complete service for exploration. Fixed finite population is
 // revisited within N waves, independent of its scoring heuristic.
 const auto explore=cursor%bids.size();take(explore);cursor=(explore+1)%bids.size();
 for(int bucket=15;bucket>=0;--bucket)for(std::size_t i=0;i<bids.size();++i){
  const auto priority=std::min<u64>(15,u64(bids[i].value)/bids[i].cost+bids[i].debt);
  if(!served[i]&&priority==unsigned(bucket)&&bids[i].cost<=budget-out.spent)take(i);
 }
 for(std::size_t i=0;i<bids.size();++i){if(served[i])bids[i].debt=0;else{if(bids[i].debt<std::numeric_limits<unsigned>::max())++bids[i].debt;out.deferred.push_back(i);}}
 return out;
}
// E28: fixed context-tag Jacobi fixture. Fine support survives promotion.
struct Cell {Support source;u64 context=0,context_version=0;unsigned stable_rounds=0;bool active=true,promoted=false;std::set<u64> message_sources;};
struct Message {std::size_t target;u64 from_source,tag,expected_context_version;};
inline void wake(std::vector<Cell>& cells,const Message& message){
 bounded(cells.size());if(message.target>=cells.size())throw std::out_of_range("message target");auto& cell=cells[message.target];
 if(cell.context_version!=message.expected_context_version)throw std::invalid_argument("stale context");
 const bool new_provenance=cell.message_sources.insert(message.from_source).second;
 if((cell.context|message.tag)!=cell.context||new_provenance){if(cell.context_version==std::numeric_limits<u64>::max())throw std::overflow_error("context version");
  cell.context|=message.tag;++cell.context_version;cell.active=true;cell.promoted=false;cell.stable_rounds=0;
  for(auto neighbor:{message.target?message.target-1:message.target,message.target+1<cells.size()?message.target+1:message.target}){cells[neighbor].active=true;cells[neighbor].promoted=false;cells[neighbor].stable_rounds=0;}}
}
struct Wavefront {std::size_t updated=0,deferred=0;};
inline Wavefront jacobi(std::vector<Cell>& cells,std::size_t capacity){
 if(cells.size()>64)throw std::invalid_argument("local cell fixture limit");bounded(capacity);
 const auto previous=cells;Wavefront counts;
 for(std::size_t i=0;i<cells.size();++i){if(!previous[i].active)continue;if(counts.updated>=capacity){++counts.deferred;continue;}++counts.updated;
  u64 context=previous[i].context;auto lineage=previous[i].message_sources;
  for(auto j:{i?i-1:i,i+1<cells.size()?i+1:i}){context|=previous[j].context;lineage.insert(previous[j].message_sources.begin(),previous[j].message_sources.end());}
  auto& cell=cells[i];const bool changed=context!=previous[i].context||lineage!=previous[i].message_sources;
  if(changed){if(cell.context_version==std::numeric_limits<u64>::max())throw std::overflow_error("context version");cell.context=context;cell.message_sources=std::move(lineage);++cell.context_version;cell.stable_rounds=0;cell.promoted=false;}
  else cell.stable_rounds=std::min(2u,cell.stable_rounds+1);
  cell.active=changed||cell.stable_rounds<2;cell.promoted=!cell.active;
 }
 // A changed neighbor can wake a previously inactive fine cell on the next wave.
 for(std::size_t i=0;i<cells.size();++i)if(cells[i].context!=previous[i].context||cells[i].message_sources!=previous[i].message_sources){
  for(auto j:{i?i-1:i,i+1<cells.size()?i+1:i}){cells[j].active=true;cells[j].promoted=false;}}
 return counts;
}
} // namespace bp_moon::demand
