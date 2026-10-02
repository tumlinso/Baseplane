#pragma once
#include <bp_moon/source.hpp>
#include <ce_moon/tensor.hpp>
namespace bp_moon::tensor {
namespace numerical=ce_moon::tensor;
// Sequence is presented in SourceMap.strand orientation. Coordinates map this
// orientation; no implicit reverse complement or inferred strand is performed.
struct Window {u64 id;PackedFixture sequence;SourceMap source;};
struct Grounding {u64 id;SourceMap source;std::vector<u64> coordinates;std::vector<unsigned char> valid;std::string exact;};
struct Cohort {numerical::Matrix16 features{};numerical::Ids16 ids{};std::vector<Grounding> grounding;};
inline unsigned base_code(char c){switch(c){case 'A':case 'a':return 0;case 'C':case 'c':return 1;case 'G':case 'g':return 2;case 'T':case 't':return 3;default:throw std::invalid_argument("invalid sequence payload");}}
inline Cohort pack_windows(const std::vector<Window>& windows){
 if(windows.empty()||windows.size()>16)throw std::invalid_argument("cohort rows must be 1..16");Cohort out;
 for(unsigned row=0;row<windows.size();++row){const auto& w=windows[row];if(w.source.length!=w.sequence.original.size()||!w.source.length)throw std::invalid_argument("window source length");
  if(w.source.length>(1u<<24))throw std::invalid_argument("window too long for exact FP32 counts");
  for(unsigned previous=0;previous<row;++previous)if(windows[previous].id==w.id)throw std::invalid_argument("duplicate object ID");
  Grounding g{w.id,w.source,{},{},w.sequence.original};out.ids[row]=w.id;
  for(std::size_t p=0;p<w.source.length;++p){bool valid=w.sequence.is_valid(p);g.coordinates.push_back(w.source.coordinate(p));g.valid.push_back(valid);
   if(valid)out.features[row*16+base_code(w.sequence.original[p])]+=1.f;}
  // Counts are exact in this bounded fixture domain; raw sequence kept for loss recovery.
  out.grounding.push_back(std::move(g));
 }return out;
}
inline bool fully_valid(const Grounding& g){return std::all_of(g.valid.begin(),g.valid.end(),[](auto x){return x!=0;});}
struct Promoted {numerical::ObjectTile latent;std::vector<Grounding> grounding;};
inline Promoted promote(const Cohort& input,const numerical::Matrix16& transform,unsigned outputs){return {numerical::object_features(input.features,transform,{unsigned(input.grounding.size()),4,outputs},input.ids),input.grounding};}
struct GroundedPair {Grounding from,to;float score;};
struct Directed {std::vector<GroundedPair> pairs;std::size_t required;bool overflow;};
inline Directed nominate(const Cohort& input,float threshold,std::size_t capacity){
 unsigned rows=input.grounding.size();numerical::Matrix16 q{},k{};numerical::PairMask mask{};
 for(unsigned i=0;i<rows;++i){q[i*16]=input.features[i*16+2];q[i*16+1]=input.features[i*16+1];k[i*16]=input.features[i*16];k[i*16+1]=input.features[i*16+3];
  for(unsigned j=0;j<rows;++j)mask[i*16+j]=i!=j&&fully_valid(input.grounding[i])&&fully_valid(input.grounding[j])&&input.grounding[i].source.strand==input.grounding[j].source.strand;}
 auto scores=numerical::relation_scores(q,k,rows,2);auto emitted=numerical::compact_relations(scores,input.ids,rows,mask,threshold,capacity);Directed out{{},emitted.required,emitted.overflow};
 for(auto p:emitted.pairs)out.pairs.push_back({input.grounding[p.row],input.grounding[p.column],p.score});return out;
}
// Sixteen states encode the last two canonical bases. Missing payload has no
// invented biological transition: an invalid chunk must be revisited/refined.
inline ce_moon::Relation16 chunk_relation(const PackedFixture& sequence){
 if(sequence.original.empty())throw std::invalid_argument("empty chunk");ce_moon::Relation16 out{};
 for(unsigned incoming=0;incoming<16;++incoming){unsigned state=incoming;for(std::size_t p=0;p<sequence.original.size();++p){if(!sequence.is_valid(p))throw std::invalid_argument("unknown base requires refinement");state=(state*4+base_code(sequence.original[p]))&15u;}out[incoming][state]=1;}return out;
}
struct FiniteResponse {ce_moon::Relation16 relation;Grounding left,right;};
inline FiniteResponse concatenate_response(const Window& left,const Window& right){
 if(left.source.source_id!=right.source.source_id||left.source.contig!=right.source.contig||left.source.version!=right.source.version||left.source.strand!=right.source.strand)throw std::invalid_argument("incompatible chunk sources");
 auto grounded=pack_windows({left,right});auto& a=grounded.grounding[0];auto& b=grounded.grounding[1];
 u64 boundary=a.coordinates.back();if(left.source.strand==Strand::forward){if(boundary==std::numeric_limits<u64>::max()||b.coordinates.front()!=boundary+1)throw std::invalid_argument("nonadjacent forward chunks");}else{if(!boundary||b.coordinates.front()!=boundary-1)throw std::invalid_argument("nonadjacent reverse chunks");}
 return {numerical::existence(numerical::finite_relation_counts(chunk_relation(left.sequence),chunk_relation(right.sequence))),a,b};
}
struct RegionalTable {numerical::Matrix16 response{},weights{};std::array<float,16> coordinates{};Grounding grounding;unsigned rows;};
inline RegionalTable sample_region(const Window& region,const std::vector<float>& entries){
 auto cohort=pack_windows({region});if(!fully_valid(cohort.grounding[0]))throw std::invalid_argument("unknown region requires refinement");if(entries.empty()||entries.size()>16)throw std::invalid_argument("state samples 1..16");
 RegionalTable out;out.grounding=cohort.grounding[0];out.rows=entries.size();numerical::Matrix16 states{};
 float gc=(cohort.features[1]+cohort.features[2])/float(region.source.length);out.weights[0]=1;out.weights[16]=gc;
 for(unsigned i=0;i<entries.size();++i){if(!std::isfinite(entries[i])||(i&&entries[i]<=entries[i-1]))throw std::invalid_argument("entry coordinates");out.coordinates[i]=entries[i];states[i*16]=entries[i];states[i*16+1]=1;}
 out.response=numerical::possible_states(states,out.weights,{out.rows,2,1});return out;
}
struct RegionalQuery {float approximate,direct,error;Grounding revisit;};
inline RegionalQuery query_region(const RegionalTable& table,float entry){
 auto interpolated=numerical::interpolate(table.response,table.coordinates,table.rows,1,entry);numerical::Matrix16 state{};state[0]=entry;state[1]=1;
 float direct=numerical::possible_states(state,table.weights,{1,2,1})[0];return {interpolated.values[0],direct,std::abs(interpolated.values[0]-direct),table.grounding};
}
} // namespace bp_moon::tensor
