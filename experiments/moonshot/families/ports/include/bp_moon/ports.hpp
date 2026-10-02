#pragma once
#include <bp_moon/source.hpp>
#include <ce_moon/mechanisms.hpp>
#include <array>
#include <set>
namespace bp_moon::ports {
namespace numeric=ce_moon::mechanisms;
struct Site {
  u64 source_id,contig,coordinate,version;
  Strand strand;
  std::size_t position;
  char exact_base;
};
inline unsigned code(char base) {
  switch(base) {case 'A':case 'a':return 0;case 'C':case 'c':return 1;
    case 'G':case 'g':return 2;case 'T':case 't':return 3;default:throw std::invalid_argument("invalid base feature");}
}
inline void validate(const PackedFixture& sequence,const SourceMap& source) {
  if(source.length!=sequence.original.size()) throw std::invalid_argument("source length");
  if(source.length>128) throw std::invalid_argument("consumer fixture limit is 128 positions");
  if(source.strand!=Strand::forward && source.strand!=Strand::reverse) throw std::invalid_argument("strand");
  if(source.length) { (void)source.coordinate(0); (void)source.coordinate(source.length-1); }
}
inline Site site_at(const PackedFixture& sequence,const SourceMap& source,std::size_t position) {
  validate(sequence,source);
  if(!sequence.is_valid(position)) throw std::invalid_argument("invalid source site");
  return {source.source_id,source.contig,source.coordinate(position),source.version,source.strand,position,sequence.original[position]};
}
inline std::vector<Site> valid_sites(const PackedFixture& sequence,const SourceMap& source) {
  validate(sequence,source);std::vector<Site> sites;
  for(std::size_t i=0;i<sequence.original.size();++i)if(sequence.is_valid(i))sites.push_back(site_at(sequence,source,i));
  return sites;
}
// Tiny synthetic model built from valid base values and actual contiguous adjacency only.
// Invalid positions break adjacency. This model supplies no biological regional annotation.
struct SequenceModel { std::vector<Site> sites; numeric::Matrix coupling; std::vector<double> load; };
inline SequenceModel sequence_model(const PackedFixture& sequence,const SourceMap& source) {
  auto sites=valid_sites(sequence,source);std::size_t n=sites.size();
  if(n==0 || n>128) throw std::invalid_argument("model requires 1..128 valid sites");
  SequenceModel model{sites,numeric::Matrix(n,n),std::vector<double>(n)};
  for(std::size_t i=0;i<n;++i) {
    unsigned base=code(sites[i].exact_base);model.coupling(i,i)=2+double(base)/4;model.load[i]=base+1;
    if(i && sites[i].position==sites[i-1].position+1)model.coupling(i,i-1)=model.coupling(i-1,i)=-0.25;
  }
  return model;
}
struct RegionPorts {
  SequenceModel model;
  std::array<std::size_t,2> port_rows;
  std::vector<std::size_t> interior_rows;
  numeric::PortResponse response;
};
inline RegionPorts expose_ports(const PackedFixture& sequence,const SourceMap& source) {
  auto model=sequence_model(sequence,source);const std::size_t n=model.sites.size();
  if(n<3)throw std::invalid_argument("ports require two boundary and one interior valid site");
  std::array<std::size_t,2> boundary{0,n-1};std::vector<std::size_t> interior;
  for(std::size_t i=1;i+1<n;++i)interior.push_back(i);
  numeric::Matrix app(2,2),api(2,n-2),aip(n-2,2),aii(n-2,n-2);
  std::vector<double> bp(2),bi(n-2);
  for(std::size_t p=0;p<2;++p) {
    bp[p]=model.load[boundary[p]];
    for(std::size_t q=0;q<2;++q)app(p,q)=model.coupling(boundary[p],boundary[q]);
    for(std::size_t j=0;j<n-2;++j) {
      api(p,j)=model.coupling(boundary[p],interior[j]);aip(j,p)=model.coupling(interior[j],boundary[p]);
    }
  }
  for(std::size_t i=0;i<n-2;++i) {
    bi[i]=model.load[interior[i]];
    for(std::size_t j=0;j<n-2;++j)aii(i,j)=model.coupling(interior[i],interior[j]);
  }
  auto response=numeric::condense_ports(app,api,aip,aii,bp,bi);
  return {std::move(model),boundary,std::move(interior),std::move(response)};
}
struct SiteValue { Site support; double value; };
inline std::vector<SiteValue> reconstruct_region(const RegionPorts& region) {
  auto port=numeric::solve_ports(region.response),inside=numeric::reconstruct_interior(region.response,port);
  std::vector<SiteValue> out(region.model.sites.size());
  for(std::size_t p=0;p<2;++p)out[region.port_rows[p]]={region.model.sites[region.port_rows[p]],port[p]};
  for(std::size_t i=0;i<inside.size();++i)out[region.interior_rows[i]]={region.model.sites[region.interior_rows[i]],inside[i]};
  return out;
}
struct CoarseSequence {
  SequenceModel model;
  std::vector<std::size_t> bin_of_site;
  numeric::Matrix prolongation,restriction;
  numeric::StepFit fit;
  std::vector<SiteValue> corrected;
};
// Fixed position bins are an explicit experiment variant, not a learned hierarchy.
inline CoarseSequence correct_sequence(const PackedFixture& sequence,const SourceMap& source,std::size_t width) {
  if(!width)throw std::invalid_argument("zero bin width");
  auto model=sequence_model(sequence,source);std::map<std::size_t,std::size_t> bins;
  std::vector<std::size_t> membership;std::vector<std::size_t> counts;
  for(const auto& site:model.sites) {
    auto key=site.position/width;auto found=bins.find(key);
    if(found==bins.end()){std::size_t index=bins.size();found=bins.emplace(key,index).first;counts.push_back(0);}
    membership.push_back(found->second);++counts[found->second];
  }
  std::size_t n=model.sites.size(),c=counts.size();numeric::Matrix p(n,c),r(c,n);
  for(std::size_t i=0;i<n;++i){p(i,membership[i])=1;r(membership[i],i)=1.0/counts[membership[i]];}
  std::vector<double> initial(n,0);auto fit=numeric::fit_step_size(model.coupling,p,r,initial,model.load);
  auto corrected=numeric::coarse_correct(model.coupling,p,r,initial,model.load,fit.alpha);
  std::vector<SiteValue> output;for(std::size_t i=0;i<n;++i)output.push_back({model.sites[i],corrected[i]});
  return {std::move(model),membership,p,r,fit,std::move(output)};
}
// Role compatibility is the exact preceding canonical base, discovered over the whole fixture.
struct RolePosting { numeric::RoleEntry numeric_entry; Site site; Site preceding_site; };
struct SequenceFactor { std::array<Site,3> roles; std::array<Site,3> context; u64 key;double score; };
struct FactorResult { std::vector<SequenceFactor> stored;numeric::JoinResult counts;std::vector<numeric::FactorAggregate> aggregates; };
inline FactorResult discover_factors(const PackedFixture& sequence,const SourceMap& source,std::size_t capacity) {
  validate(sequence,source);
  std::array<std::vector<numeric::RoleEntry>,3> numeric_postings;
  std::map<u64,RolePosting> lookup;
  for(std::size_t i=1;i<sequence.original.size();++i) {
    if(!sequence.is_valid(i)||!sequence.is_valid(i-1))continue;
    unsigned role=code(sequence.original[i]);if(role>2)continue;
    RolePosting posting{{u64(i),code(sequence.original[i-1]),double(role+1)},site_at(sequence,source,i),site_at(sequence,source,i-1)};
    numeric_postings[role].push_back(posting.numeric_entry);lookup.emplace(u64(i),posting);
  }
  auto required=numeric::join_factors(numeric_postings[0],numeric_postings[1],numeric_postings[2],{0,1,2,3},nullptr,0).required;
  std::size_t limit=std::min(capacity,required);std::vector<numeric::FactorTuple> tuples(limit);
  auto counts=numeric::join_factors(numeric_postings[0],numeric_postings[1],numeric_postings[2],{0,1,2,3},tuples.data(),limit);
  std::vector<SequenceFactor> output;
  for(const auto& tuple:tuples) {
    SequenceFactor factor{};factor.key=tuple.key;factor.score=tuple.score;
    for(std::size_t role=0;role<3;++role){const auto& posting=lookup.at(tuple.ids[role]);factor.roles[role]=posting.site;factor.context[role]=posting.preceding_site;}
    output.push_back(factor);
  }
  return {std::move(output),counts,numeric::aggregate_factors(numeric_postings[0],numeric_postings[1],numeric_postings[2],{0,1,2,3})};
}
struct BaseChange { std::size_t position;char replacement; };
struct Alternative { u64 world_id;std::vector<BaseChange> changes; };
struct WorldSupport { u64 world_id;std::string exact_sequence;std::vector<Site> valid_support; };
struct SequenceWorlds { std::vector<WorldSupport> support;numeric::WorldBatch numerical;std::size_t query_node;std::vector<std::size_t> query_groups; };
inline SequenceWorlds alternative_worlds(const PackedFixture& baseline,const SourceMap& source,
                                       const std::vector<Alternative>& alternatives) {
  validate(baseline,source);std::size_t n=baseline.original.size();
  if(!n || n>128)throw std::invalid_argument("world fixture requires 1..128 positions");
  std::vector<numeric::Node> dag;
  for(std::size_t i=0;i<n;++i)dag.push_back({-1,-1,0,0,baseline.is_valid(i)?double(code(baseline.original[i])+1):0});
  std::size_t query=0;
  for(std::size_t i=1;i<n;++i){dag.push_back({int(query),int(i),1,1,0});query=dag.size()-1;}
  std::set<u64> world_ids;std::vector<WorldSupport> support;std::vector<std::vector<numeric::WorldDelta>> deltas;
  for(const auto& alternative:alternatives) {
    if(!world_ids.insert(alternative.world_id).second)throw std::invalid_argument("duplicate world ID");
    auto exact=baseline.original;std::set<std::size_t> changed;std::vector<numeric::WorldDelta> delta;
    for(const auto& change:alternative.changes) {
      if(change.position>=n || !changed.insert(change.position).second)throw std::invalid_argument("world position");
      exact[change.position]=change.replacement;
    }
    PackedFixture world(exact);
    for(auto position:changed)delta.push_back({position,world.is_valid(position)?double(code(exact[position])+1):0});
    support.push_back({alternative.world_id,exact,valid_sites(world,source)});deltas.push_back(std::move(delta));
  }
  auto numerical=numeric::evaluate_worlds(dag,deltas);auto groups=numeric::exact_query_groups(numerical.states,{query});
  return {std::move(support),std::move(numerical),query,std::move(groups)};
}
} // namespace bp_moon::ports
