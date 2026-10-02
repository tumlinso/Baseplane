#pragma once
#include <bp_moon/source.hpp>
#include <learning.hpp>

namespace bp_moon::compiler {
namespace ce = ce_moon::learning;
struct Triplet { bool gc, previous_a, next_t, valid; unsigned row; };
inline unsigned base_code(const PackedFixture& s, std::size_t i) {
  return unsigned((s.words.at(i/32)>>(2*(i%32)))&3u);
}
// No invalid/tail payload or missing halo is a valid triplet feature.
inline Triplet features(const PackedFixture& s, std::size_t i) {
  if(!i || i>=s.original.size() || i+1>=s.original.size() ||
     !s.is_valid(i-1) || !s.is_valid(i) || !s.is_valid(i+1)) return {false,false,false,false,0};
  unsigned code=base_code(s,i);
  bool gc=code==1 || code==2, prev=base_code(s,i-1)==0, next=base_code(s,i+1)==3;
  return {gc,prev,next,true,unsigned(4*gc+2*prev+next)};
}
inline bool scalar_question(const Triplet& x) { return x.valid && ((x.gc && x.previous_a) || x.next_t); }
inline std::array<double,8> training_labels() {
  std::array<double,8> targets{};
  for(unsigned row=0;row<8;++row) {
    std::string triplet;
    triplet += (row&2)?'A':'C'; triplet += (row&4)?'C':'A'; triplet += (row&1)?'T':'G';
    PackedFixture sequence(triplet);
    auto x=features(sequence,1);
    if(x.row!=row || !x.valid) throw std::logic_error("sequence feature table");
    targets[row]=scalar_question(x);
  }
  return targets;
}
struct FeaturePlanes { u32 gc=0, previous_a=0, next_t=0, valid=0; };
inline FeaturePlanes planes(const PackedFixture& s, std::size_t word) {
  if(word>=s.words.size()) throw std::out_of_range("feature word");
  FeaturePlanes out;
  for(unsigned lane=0;lane<32;++lane) {
    auto x=features(s,word*32+lane);
    if(!x.valid) continue;
    const auto bit=u32{1}<<lane; out.valid|=bit;
    if(x.gc)out.gc|=bit;
    if(x.previous_a)out.previous_a|=bit;
    if(x.next_t)out.next_t|=bit;
  }
  return out;
}
inline u32 hard_decisions(const FeaturePlanes& x,const ce::HardLut& lut) {
  return lut3(x.gc,x.previous_a,x.next_t,lut.immediate)&x.valid;
}
struct Provenance {
  u64 weights=0,circuit=0,query=0,state=0;
  std::size_t bank=0;
};
struct Hit {
  u64 source_id=0,contig=0,source_version=0,anchor=0;
  Strand strand=Strand::forward;
  std::array<u64,3> support{};
  unsigned support_count=0;
  Provenance provenance;
};
inline bool same_source(const SourceMap& a,const SourceMap& b) {
  return a.source_id==b.source_id && a.contig==b.contig && a.origin==b.origin &&
         a.version==b.version && a.strand==b.strand && a.length==b.length;
}
inline void validate_output(const PackedFixture& s,const SourceMap& source,Hit* output,std::size_t capacity) {
  if(source.length!=s.original.size() || (capacity && !output)) throw std::invalid_argument("source/output contract");
}
inline Hit hit(const SourceMap& source,std::size_t anchor,std::size_t first,unsigned width,Provenance p) {
  if(width>3 || !width) throw std::invalid_argument("bounded witness width");
  Hit h{source.source_id,source.contig,source.version,source.coordinate(anchor),source.strand,{},width,p};
  for(unsigned j=0;j<width;++j)h.support[j]=source.coordinate(first+j);
  return h;
}
inline void append(const Hit& h,Hit* output,std::size_t capacity,EmitCounts& counts) {
  if(counts.produced==std::numeric_limits<u64>::max())throw std::overflow_error("hit count");
  ++counts.produced;
  if(counts.stored<capacity)output[counts.stored++]=h; else ++counts.dropped;
}
// E29 packed hardening consumer; retained source supports exact later replay.
inline EmitCounts hardened_query(const PackedFixture& s,const SourceMap& source,
                                const ce::HardLut& lut,Provenance provenance,
                                Hit* output,std::size_t capacity) {
  validate_output(s,source,output,capacity);
  if(provenance.weights!=lut.teacher_version || provenance.circuit!=lut.circuit_version)
    throw std::invalid_argument("hard circuit provenance");
  EmitCounts counts;
  for(std::size_t w=0;w<s.words.size();++w) {
    u32 selected=hard_decisions(planes(s,w),lut);
    for(unsigned lane=0;lane<32;++lane)if(selected&(u32{1}<<lane)) {
      auto i=w*32+lane; append(hit(source,i,i-1,3,provenance),output,capacity,counts);
    }
  }
  return counts;
}
// E30 immutable exact predicate identities; learned scores only select the bank.
struct BankModel { std::array<double,4> weights{}; u64 version=0; };
inline BankModel train_banks(u64 version=1) {
  const double states[]{-2,-1,1,2},ac[]{1,1,0,0},gt[]{0,0,1,1};
  BankModel model; model.version=version;
  ce::fit_logistic(states,ac,4,1,model.weights.data());
  ce::fit_logistic(states,gt,4,1,model.weights.data()+2);
  return model;
}
struct BankResult { EmitCounts counts; ce::BankSelection selection; };
inline BankResult bank_query(const PackedFixture& s,const SourceMap& source,const BankModel& model,
                             double state,u64 state_version,Hit* output,std::size_t capacity) {
  validate_output(s,source,output,capacity);
  auto selected=ce::select_bank(&state,1,model.weights.data(),2,state_version,model.version);
  BankResult result{{},selected};
  const unsigned first=selected.index==0?0:2,second=selected.index==0?1:3;
  for(std::size_t i=0;i+1<s.original.size();++i) {
    if(!s.is_valid(i)||!s.is_valid(i+1))continue;
    if(base_code(s,i)==first && base_code(s,i+1)==second)
      append(hit(source,i,i,2,{model.version,0,0,state_version,selected.index}),output,capacity,result.counts);
  }
  return result;
}
// E31 rewriting remains CE-owned; Baseplane binds a particular sequence question
// and coordinate-domain identity to it, then applies its exact LUT to bitplanes.
struct CompiledQuestion { ce::RewriteResult rewrite; unsigned lut; u64 domain; };
inline CompiledQuestion compile_question(u64 domain) {
  auto a=ce::input(0,domain),b=ce::input(1,domain),c=ce::input(2,domain);
  auto expression=ce::binary(ce::MaskOp::disjunction,
    ce::binary(ce::MaskOp::conjunction,a,b),ce::binary(ce::MaskOp::conjunction,a,c));
  auto rewrite=ce::minimize_boolean(expression,3);
  return {rewrite,ce::truth_table(rewrite.expression),domain};
}
inline u32 rewritten_mask(const CompiledQuestion& question,const FeaturePlanes& x,u64 domain) {
  if(question.domain!=domain)throw std::invalid_argument("query source domain");
  return lut3(x.gc,x.previous_a,x.next_t,question.lut)&x.valid;
}
inline EmitCounts rewritten_query(const CompiledQuestion& question,const PackedFixture& s,
                                  const SourceMap& source,u64 domain,u64 query_version,
                                  Hit* output,std::size_t capacity) {
  validate_output(s,source,output,capacity);
  EmitCounts counts;
  for(std::size_t w=0;w<s.words.size();++w) {
    u32 selected=rewritten_mask(question,planes(s,w),domain);
    for(unsigned lane=0;lane<32;++lane)if(selected&(u32{1}<<lane)) {
      auto i=w*32+lane;append(hit(source,i,i-1,3,{0,0,query_version,0,0}),output,capacity,counts);
    }
  }
  return counts;
}
// E32 cached computation, not cached biological answers. Source identity checks
// supplement CE guards. Fresh features are always extracted from the supplied source.
struct PreparedQuery { ce::Specialization numerical; SourceMap source; };
inline PreparedQuery prepare(const SourceMap& source,const ce::TruthTeacher& teacher,
                             u64 query,u64 state,u64 circuit=1) {
  return {ce::specialize(teacher,{teacher.weight_version,query,state,source.version},circuit),source};
}
struct GuardedResult { EmitCounts counts; bool specialization_reused; u64 source_revisits; };
inline GuardedResult guarded_query(const PreparedQuery& prepared,const PackedFixture& s,
                                   const SourceMap& source,const ce::TruthTeacher& current,
                                   u64 query,u64 state,Hit* output,std::size_t capacity) {
  validate_output(s,source,output,capacity);
  ce::Guard context{current.weight_version,query,state,source.version};
  bool reusable=same_source(prepared.source,source) && prepared.numerical.guard==context &&
                prepared.numerical.lut.teacher_version==current.weight_version;
  GuardedResult result{{},reusable,0};
  for(std::size_t i=0;i<s.original.size();++i) {
    auto x=features(s,i); if(!x.valid)continue;
    bool decision;
    if(reusable)decision=ce::guarded_predict(prepared.numerical,current,context,x.row).decision;
    else { decision=current.predict(x.row)>=0.5; ++result.source_revisits; }
    if(decision)append(hit(source,i,i-1,3,{current.weight_version,reusable?prepared.numerical.lut.circuit_version:0,query,state,0}),output,capacity,result.counts);
  }
  return result;
}
} // namespace bp_moon::compiler
