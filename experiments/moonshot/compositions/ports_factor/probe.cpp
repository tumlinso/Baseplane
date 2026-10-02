#include <bp_moon/ports.hpp>
#include <iostream>
#include <tuple>
namespace bp=bp_moon;
namespace p=bp_moon::ports;
namespace ce=ce_moon::mechanisms;
using Signature=std::tuple<std::size_t,std::size_t,std::size_t,bp::u64>;
static void check(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
static void near(double a,double b){check(std::abs(a-b)<1e-9,"C06 numerical oracle mismatch");}
template<class F>static void rejects(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught,"expected rejection");}
static Signature signature(const p::SequenceFactor& f){return {f.roles[0].position,f.roles[1].position,f.roles[2].position,f.key};}
struct ScoredRegion {
  std::string exact_sequence;
  p::RegionPorts region;
  std::vector<p::SiteValue> responses;
  std::vector<p::SequenceFactor> factors;
  std::size_t required=0,initial_written=0;
  bool refined=false;
};
// Connected path: actual reconstructed numerical site responses become each role's scoring value.
static ScoredRegion port_to_factors(const std::string& exact,const bp::SourceMap& source,std::size_t initial_capacity) {
  bp::PackedFixture fixture(exact);
  auto region=p::expose_ports(fixture,source);auto responses=p::reconstruct_region(region);
  auto candidates=p::discover_factors(fixture,source,initial_capacity);
  auto initial_written=candidates.counts.written;bool refined=candidates.counts.overflow;
  if(refined){
    if(candidates.counts.required>64)throw std::length_error("C06 bounded factor refinement limit");
    candidates=p::discover_factors(fixture,source,candidates.counts.required);
  }
  check(!candidates.counts.overflow,"refinement must supply complete candidates");
  if(candidates.counts.required>64)throw std::length_error("C06 bounded factor limit");
  std::map<std::size_t,double> response_by_position;
  for(const auto& response:responses)response_by_position.emplace(response.support.position,response.value);
  std::array<std::map<bp::u64,ce::RoleEntry>,3> roles;
  std::map<Signature,p::SequenceFactor> provenance;
  for(const auto& factor:candidates.stored){
    provenance.emplace(signature(factor),factor);
    for(std::size_t role=0;role<3;++role){
      const auto& site=factor.roles[role];
      roles[role].emplace(site.position,ce::RoleEntry{site.position,factor.key,response_by_position.at(site.position)});
    }
  }
  std::array<std::vector<ce::RoleEntry>,3> postings;
  for(std::size_t role=0;role<3;++role)for(const auto& item:roles[role])postings[role].push_back(item.second);
  const std::vector<double> weights{0.25,1,0.5,-0.25};
  auto count=ce::join_factors(postings[0],postings[1],postings[2],weights,nullptr,0);
  check(count.required==candidates.counts.required,"factor reconstruction changed candidate relation");
  std::vector<ce::FactorTuple> scored(count.required);
  auto result=ce::join_factors(postings[0],postings[1],postings[2],weights,scored.data(),scored.size());
  check(!result.overflow,"scored factors complete");
  std::vector<p::SequenceFactor> output;
  for(const auto& tuple:scored){
    Signature id{std::size_t(tuple.ids[0]),std::size_t(tuple.ids[1]),std::size_t(tuple.ids[2]),tuple.key};
    auto factor=provenance.at(id);factor.score=tuple.score;output.push_back(factor);
  }
  return {exact,std::move(region),std::move(responses),std::move(output),count.required,initial_written,refined};
}
// Independent tiny sequence oracle: full solve and direct Cartesian context comparison.
// This uses the provider's solve, with a fixture-local scalar score expression for comparison.
static std::vector<std::pair<Signature,double>> naive_factors(const std::string& exact,const bp::SourceMap& source) {
  bp::PackedFixture fixture(exact);auto model=p::sequence_model(fixture,source);
  auto full=ce::solve(model.coupling,model.load);std::map<std::size_t,double> values;
  for(std::size_t i=0;i<model.sites.size();++i)values.emplace(model.sites[i].position,full[i]);
  std::array<std::vector<std::size_t>,3> roles;
  for(std::size_t position=1;position<exact.size();++position){
    if(!fixture.is_valid(position)||!fixture.is_valid(position-1))continue;
    auto role=p::code(exact[position]);if(role<3)roles[role].push_back(position);
  }
  std::vector<std::pair<Signature,double>> result;
  for(auto a:roles[0])for(auto b:roles[1])for(auto c:roles[2]){
    auto key=p::code(exact[a-1]);
    if(key!=p::code(exact[b-1]) || key!=p::code(exact[c-1]))continue;
    result.push_back({Signature{a,b,c,key},0.25+values.at(a)+0.5*values.at(b)-0.25*values.at(c)});
  }
  return result;
}
static std::string apply_alternative(const std::string& baseline,const p::Alternative& alternative){
  auto exact=baseline;std::set<std::size_t> changed;
  for(const auto& change:alternative.changes){
    if(change.position>=exact.size() || !changed.insert(change.position).second)
      throw std::invalid_argument("invalid/duplicate alternative coordinate");
    exact[change.position]=change.replacement;
  }
  return exact;
}
struct FactorWorlds {
  std::vector<bp::u64> world_ids;
  std::vector<ScoredRegion> regions;
  ce::WorldBatch numerical;
  std::vector<std::size_t> query_groups;
  std::vector<bp::u64> rebuild_world_ids;
  std::vector<double> rebuilt_queries;
  std::size_t query_node=0;
};
// Exact factor topology is guarded before constructing fixed-DAG leaf deltas.
static FactorWorlds factor_worlds(const std::string& baseline,const bp::SourceMap& source,
                                 const std::vector<p::Alternative>& alternatives,std::size_t capacity){
  auto base=port_to_factors(baseline,source,capacity);
  if(base.factors.empty())throw std::invalid_argument("C06 baseline has no factors");
  std::vector<ce::Node> dag;
  for(const auto& factor:base.factors)dag.push_back({-1,-1,0,0,factor.score});
  std::size_t query=0;
  for(std::size_t i=1;i<base.factors.size();++i){dag.push_back({int(query),int(i),1,1,0});query=dag.size()-1;}
  FactorWorlds result;result.query_node=query;
  std::set<bp::u64> ids;std::vector<std::vector<ce::WorldDelta>> deltas;
  for(const auto& alternative:alternatives){
    if(!ids.insert(alternative.world_id).second)throw std::invalid_argument("duplicate C06 world ID");
    auto region=port_to_factors(apply_alternative(baseline,alternative),source,capacity);
    bool same=region.factors.size()==base.factors.size();
    if(same)for(std::size_t i=0;i<base.factors.size();++i)
      if(signature(base.factors[i])!=signature(region.factors[i])){same=false;break;}
    if(!same){
      // Rebuild rather than attaching changed roles to an incompatible cached graph.
      std::vector<ce::Node> rebuilt;
      for(const auto& factor:region.factors)rebuilt.push_back({-1,-1,0,0,factor.score});
      std::size_t rebuilt_query=0;
      for(std::size_t i=1;i<region.factors.size();++i){rebuilt.push_back({int(rebuilt_query),int(i),1,1,0});rebuilt_query=rebuilt.size()-1;}
      double answer=rebuilt.empty()?0:ce::evaluate_independent(rebuilt)[rebuilt_query];
      result.rebuild_world_ids.push_back(alternative.world_id);result.rebuilt_queries.push_back(answer);
    }else{
      std::vector<ce::WorldDelta> delta;
      for(std::size_t i=0;i<base.factors.size();++i)if(region.factors[i].score!=base.factors[i].score)
        delta.push_back({i,region.factors[i].score});
      result.world_ids.push_back(alternative.world_id);deltas.push_back(std::move(delta));
    }
    result.regions.push_back(std::move(region));
  }
  result.numerical=ce::evaluate_worlds(dag,deltas);
  result.query_groups=ce::exact_query_groups(result.numerical.states,{query});
  return result;
}
static void validate_provenance(const ScoredRegion& region,const bp::SourceMap& source){
  bp::PackedFixture fixture(region.exact_sequence);
  for(const auto& factor:region.factors)for(std::size_t role=0;role<3;++role){
    for(const auto& site:{factor.roles[role],factor.context[role]}){
      check(site.source_id==source.source_id&&site.contig==source.contig&&site.version==source.version&&site.strand==source.strand,"C06 source identity");
      check(fixture.is_valid(site.position)&&site.exact_base==fixture.original[site.position],"C06 exact valid role source");
      check(site.coordinate==source.coordinate(site.position),"C06 coordinate source");
    }
    check(p::code(factor.context[role].exact_base)==factor.key,"C06 observed context key");
  }
}
int main(){try{
  const std::string baseline="TATCTGTATCTGTT";
  const bp::SourceMap source{7001,12,8000,3,bp::Strand::reverse,baseline.size()};
  auto first=port_to_factors(baseline,source,1);
  check(first.required==8 && first.initial_written==1 && first.refined,"C06 overflow/refinement path");
  auto naive=naive_factors(baseline,source);check(naive.size()==first.factors.size(),"C06 naive cardinality");
  for(std::size_t i=0;i<naive.size();++i){check(signature(first.factors[i])==naive[i].first,"C06 tuple oracle");near(first.factors[i].score,naive[i].second);}
  validate_provenance(first,source);
  auto unrefined=bp_moon::ports::discover_factors(bp::PackedFixture(baseline),source,1);
  check(std::abs(first.factors[0].score-unrefined.stored[0].score)>1e-3,"C06 factor value must consume reconstructed responses");
  std::vector<p::Alternative> alternatives{{10,{}},{11,{{13,'N'}}},{12,{{12,'N'}}},{13,{{12,'X'}}},{14,{{2,'N'}}}};
  auto worlds=factor_worlds(baseline,source,alternatives,1);
  check(worlds.world_ids.size()==4 && worlds.rebuild_world_ids==std::vector<bp::u64>{14},"C06 topology rebuild guard");
  for(std::size_t w=0;w<worlds.regions.size();++w){
    const auto& region=worlds.regions[w];validate_provenance(region,source);
    auto oracle=naive_factors(region.exact_sequence,source);double expected=0;
    check(oracle.size()==region.factors.size(),"C06 world role cardinality");
    for(std::size_t i=0;i<oracle.size();++i){check(signature(region.factors[i])==oracle[i].first,"C06 world tuple oracle");near(region.factors[i].score,oracle[i].second);expected+=oracle[i].second;}
    if(w<4){
      check(worlds.world_ids[w]==alternatives[w].world_id,"C06 stable world ID");
      near(worlds.numerical.states[w][worlds.query_node],expected);
      std::vector<ce::Node> independent;
      for(const auto& factor:region.factors)independent.push_back({-1,-1,0,0,factor.score});
      std::size_t query=0;for(std::size_t i=1;i<region.factors.size();++i){independent.push_back({int(query),int(i),1,1,0});query=independent.size()-1;}
      check(ce::evaluate_independent(independent)==worlds.numerical.states[w],"C06 independent world DAG");
    }else near(worlds.rebuilt_queries[0],expected);
  }
  check(worlds.regions[2].exact_sequence!=worlds.regions[3].exact_sequence,"C06 equal query different sequences");
  check(worlds.query_groups[2]==worlds.query_groups[3],"C06 query-relative equality");
  near(worlds.numerical.states[2][worlds.query_node],worlds.numerical.states[3][worlds.query_node]);
  check(worlds.numerical.states[0][worlds.query_node]!=worlds.numerical.states[1][worlds.query_node],"C06 mutation changes connected numerical result");
  rejects([&]{factor_worlds(baseline,source,{{1,{}},{1,{}}},1);});
  rejects([&]{apply_alternative(baseline,{1,{{999,'A'}}});});
  rejects([&]{apply_alternative(baseline,{1,{{2,'A'},{2,'C'}}});});
  rejects([&]{std::string large;for(const char* token:{"TA","TC","TG"})for(int i=0;i<5;++i)large+=token;
    port_to_factors(large,bp::SourceMap{1,1,0,1,bp::Strand::forward,large.size()},1);});
  std::cout<<"C06 reconstructed port -> factor -> world path: candidates=8 initial_written=1 refined=8\n";
  std::cout<<"C06 world queries=";for(const auto& state:worlds.numerical.states)std::cout<<state[worlds.query_node]<<" ";
  std::cout<<"shared evaluations="<<worlds.numerical.evaluations<<" topology-rebuild world=14\n";
  std::cout<<"C06 full-solve/brute-join/independent-world oracles agree; source roles, invalidity, reverse coordinates and alternatives retained\n";
}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}}
