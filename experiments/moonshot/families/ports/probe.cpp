#include <bp_moon/ports.hpp>
#include <iostream>
using namespace bp_moon;
namespace p=bp_moon::ports;
static void check(bool c,const char* m){if(!c)throw std::runtime_error(m);}
static void near(double a,double b){check(std::abs(a-b)<1e-9,"numeric comparison");}
template<class F>static void rejects(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught,"expected rejection");}
static void support(const p::Site& site,const SourceMap& source,const PackedFixture& sequence){
  check(site.source_id==source.source_id && site.contig==source.contig && site.version==source.version && site.strand==source.strand,"source identity");
  check(sequence.is_valid(site.position),"validity");
  check(site.exact_base==sequence.original[site.position] && site.coordinate==source.coordinate(site.position),"exact coordinate/base");
}
int main(){try{
  PackedFixture sequence("ACNTGCA");SourceMap source{71,3,1000,4,Strand::reverse,sequence.original.size()};
  auto region=p::expose_ports(sequence,source);auto reconstructed=p::reconstruct_region(region);
  auto full=p::numeric::solve(region.model.coupling,region.model.load);
  check(reconstructed.size()==6,"invalid N excluded");
  for(std::size_t i=0;i<reconstructed.size();++i){support(reconstructed[i].support,source,sequence);near(reconstructed[i].value,full[i]);}
  check(region.model.coupling(1,2)==0,"invalid gap breaks adjacency");
  check(reconstructed.front().support.coordinate==1006 && reconstructed.back().support.coordinate==1000,"reverse coordinates");
  check(region.model.sites[region.port_rows[0]].position==0 && region.model.sites[region.port_rows[1]].position==6,"port source anchors");
  rejects([&]{p::expose_ports(PackedFixture("AN"),SourceMap{1,1,0,1,Strand::forward,2});});
  std::cout<<"E41 valid=6 ports=(1006,1000) reconstruction agrees with full CE solve; invalid gap uncoupled\n";

  auto coarse=p::correct_sequence(sequence,source,3);
  check(coarse.fit.loss_after<coarse.fit.loss_before,"coarse sequence improvement");
  check(coarse.prolongation.cols==3,"three observed position bins");
  for(auto value:coarse.corrected)support(value.support,source,sequence);
  for(std::size_t c=0;c<coarse.restriction.rows;++c){double sum=0;for(std::size_t i=0;i<coarse.restriction.cols;++i)sum+=coarse.restriction(c,i);near(sum,1);}
  std::vector<double> initial(coarse.model.sites.size(),0);
  auto missed=p::numeric::coarse_correct(coarse.model.coupling,coarse.prolongation,coarse.restriction,initial,coarse.model.load,0);
  near(p::numeric::objective(coarse.model.coupling,missed,coarse.model.load),coarse.fit.loss_before);
  rejects([&]{p::correct_sequence(sequence,source,0);});
  std::cout<<"E42 observed bins=3 fitted alpha="<<coarse.fit.alpha<<" residual loss "<<coarse.fit.loss_before<<" -> "<<coarse.fit.loss_after<<"\n";

  PackedFixture role_sequence("TATCTGNACAG");SourceMap roles{82,4,9000,2,Strand::forward,role_sequence.original.size()};
  auto factors=p::discover_factors(role_sequence,roles,4);
  check(factors.counts.required==1 && factors.counts.written==1 && !factors.counts.overflow,"sparse role tuple");
  std::array<std::size_t,3> expected{1,3,5};
  for(std::size_t role=0;role<3;++role){auto s=factors.stored[0].roles[role];support(s,roles,role_sequence);support(factors.stored[0].context[role],roles,role_sequence);check(s.position==expected[role],"whole-source role join");}
  near(factors.stored[0].score,14);check(factors.stored[0].key==3,"shared preceding T key");
  check(factors.aggregates.size()==1 && factors.aggregates[0].count==1,"factorized relation");near(factors.aggregates[0].score_sum,14);
  auto overflow=p::discover_factors(role_sequence,roles,0);check(overflow.counts.required==1&&overflow.counts.written==0&&overflow.counts.overflow,"zero capacity policy");
  auto empty=p::discover_factors(PackedFixture("ANCG"),SourceMap{1,1,0,1,Strand::forward,4},10);check(empty.counts.required==0,"invalid context cannot fill roles");
  std::cout<<"E43 sequence role positions=(1,3,5) key=T score=14; N-preceded roles excluded\n";

  PackedFixture baseline("ACNT");SourceMap worlds_source{93,5,5000,9,Strand::reverse,4};
  std::vector<p::Alternative> alternatives{{10,{}},{11,{{2,'G'}}},{12,{{0,'T'}}},{13,{{3,'N'}}}};
  auto worlds=p::alternative_worlds(baseline,worlds_source,alternatives);
  check(worlds.support.size()==4,"world ID count");
  std::array<double,4> expected_outputs{7,10,10,3};
  for(std::size_t w=0;w<worlds.support.size();++w){
    check(worlds.support[w].world_id==alternatives[w].world_id,"world identity");
    PackedFixture independent(worlds.support[w].exact_sequence);double oracle=0;
    for(std::size_t i=0;i<independent.original.size();++i)if(independent.is_valid(i))oracle+=p::code(independent.original[i])+1;
    near(worlds.numerical.states[w][worlds.query_node],oracle);near(oracle,expected_outputs[w]);
    for(auto site:worlds.support[w].valid_support)support(site,worlds_source,independent);
  }
  check(worlds.query_groups[1]==worlds.query_groups[2],"exact queried equality");
  check(worlds.support[1].exact_sequence!=worlds.support[2].exact_sequence,"equivalent query retains different sequences");
  check(worlds.support[1].valid_support.size()==4 && worlds.support[2].valid_support.size()==3,"validity changes retained separately");
  check(worlds.numerical.evaluations<4*(2*baseline.original.size()-1),"shared worlds recomputation");
  rejects([&]{p::alternative_worlds(baseline,worlds_source,{{1,{}},{1,{}}});});
  rejects([&]{std::vector<p::Alternative> bad{{1,{{2,'A'},{2,'G'}}}};p::alternative_worlds(baseline,worlds_source,bad);});
  rejects([&]{std::vector<p::Alternative> bad{{1,{{4,'A'}}}};p::alternative_worlds(baseline,worlds_source,bad);});
  rejects([&]{p::valid_sites(sequence,SourceMap{1,1,0,1,Strand::forward,2});});
  std::cout<<"E44 outputs=(7,10,10,3) shared evaluations="<<worlds.numerical.evaluations<<" independent=28; equal-query worlds retain source alternatives\n";
  rejects([&]{p::valid_sites(sequence,SourceMap{1,1,std::numeric_limits<u64>::max(),1,Strand::reverse,sequence.original.size()});});
  rejects([&]{p::valid_sites(PackedFixture(std::string(129,'A')),SourceMap{1,1,0,1,Strand::forward,129});});
  auto invalid_world=p::alternative_worlds(PackedFixture("NN"),SourceMap{1,1,0,1,Strand::forward,2},{{100,{}}});
  check(invalid_world.support[0].valid_support.empty(),"no biological support from invalid payload");near(invalid_world.numerical.states[0][invalid_world.query_node],0);
  std::cout<<"BP-MOON-120 four sequence consumers compared successfully\n";
}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}}
