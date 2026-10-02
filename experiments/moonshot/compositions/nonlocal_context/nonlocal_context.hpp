#pragma once
#include "../../families/rendezvous/rendezvous.hpp"
#include "../../families/hierarchy/hierarchy.hpp"
#include "../../families/demand/demand.hpp"
#include <bp_moon/tensor.hpp>
#include <learning.hpp>
namespace bp_moon::compositions::nonlocal {
struct BoundaryModel {std::array<double,5> weights{};ce_moon::learning::FitReport fit;};
inline std::vector<double> boundary_features(const PackedFixture& s){std::vector<double> rows;for(std::size_t p=1;p<s.original.size();++p){double l=hierarchy::gc(s.original[p-1]),r=hierarchy::gc(s.original[p]);rows.insert(rows.end(),{l,r,l!=r?1.:0.,s.is_valid(p-1)&&s.is_valid(p)?1.:0.});}return rows;}
// Synthetic training only; inference below observes bases and validity, not labels.
inline BoundaryModel train_boundaries(){std::string text;for(unsigned i=0;i<24;++i)text+="AAAAAAAAGGGGGGGG";PackedFixture sequence(text);auto rows=boundary_features(sequence);std::vector<double> labels;for(std::size_t p=1;p<text.size();++p)labels.push_back(hierarchy::gc(text[p-1])!=hierarchy::gc(text[p]));BoundaryModel model;ce_moon::learning::FitOptions options;options.budget_weight=.5;options.positive_budget=.15;model.fit=ce_moon::learning::fit_logistic(rows.data(),labels.data(),labels.size(),4,model.weights.data(),options);return model;}
inline std::vector<hierarchy::Span> boundaries(const PackedFixture& sequence,const BoundaryModel& model){auto rows=boundary_features(sequence);std::vector<std::size_t> cuts;std::size_t previous=0;for(std::size_t p=1;p<sequence.original.size();++p)if(p-previous>=16||(p-previous>=2&&ce_moon::learning::predict_logistic(rows.data()+(p-1)*4,4,model.weights.data())>.5)){cuts.push_back(p);previous=p;}return hierarchy::chunks(sequence.original.size(),cuts);}
inline bool same_source(const SourceMap& a,const SourceMap& b){return a.source_id==b.source_id&&a.contig==b.contig&&a.version==b.version&&a.origin==b.origin&&a.length==b.length&&a.strand==b.strand;}
inline std::size_t owner(const std::vector<discovery::Sequence>& sources,const discovery::Object& object){for(std::size_t i=0;i<sources.size();++i)if(same_source(sources[i].source,object.source))return i;throw std::invalid_argument("stale or unavailable object source");}
struct Revisit {u64 object_id;SourceMap source;std::string exact;std::vector<u64> coordinates;};
inline Revisit revisit(const std::vector<discovery::Sequence>& sources,const discovery::Object& object){const auto& source=sources.at(owner(sources,object));if(object.local>source.source.length||object.exact.size()>source.source.length-object.local)throw std::invalid_argument("object support outside source");Revisit out{object.id,source.source,source.sequence.original.substr(object.local,object.exact.size()),{}};std::string canonical;
 for(std::size_t j=0;j<out.exact.size();++j){auto p=object.local+j;if(!source.sequence.is_valid(p))throw std::invalid_argument("changed object validity");canonical.push_back(discovery::canonical(out.exact[j]));out.coordinates.push_back(source.source.coordinate(p));}
 if(canonical!=object.exact||out.coordinates.front()!=object.first||out.coordinates.back()!=object.last)throw std::invalid_argument("changed object payload or coordinates");return out;
}
struct Edge {u64 from,to;float score;std::size_t from_source,to_source;Revisit from_support,to_support;std::vector<std::size_t> destination_chunks;};
struct Result {BoundaryModel model;std::vector<std::vector<hierarchy::Span>> chunks;std::vector<std::vector<std::size_t>> inverse;std::vector<std::vector<demand::Cell>> cells;std::vector<discovery::Object> objects;discovery::Costs costs;EmitCounts candidates;std::size_t required=0;bool overflow=false;std::vector<Edge> edges;std::size_t waves=0,updates=0,deferred_updates=0;bool converged=false;};
inline Result run(const std::vector<discovery::Sequence>& sources,std::size_t pair_capacity=256,std::size_t wave_capacity=64,unsigned wave_budget=64){
 if(sources.empty()||sources.size()>16||wave_budget>64)throw std::invalid_argument("bounded composition sources/waves");Result out;out.model=train_boundaries();
 for(std::size_t i=0;i<sources.size();++i){const auto& input=sources[i];demand::validate(input.sequence,input.source);if(!input.source.length)throw std::invalid_argument("empty source");for(std::size_t j=0;j<i;++j)if(same_source(input.source,sources[j].source))throw std::invalid_argument("duplicate source identity");
  out.chunks.push_back(boundaries(input.sequence,out.model));out.inverse.push_back(hierarchy::inverse_map(input.source.length,out.chunks.back()));std::vector<demand::Cell> cells;
  for(auto span:out.chunks.back())cells.push_back({demand::support(input.source,span.begin,span.end)});if(cells.size()>64)throw std::invalid_argument("source exceeds local cell limit");
  demand::jacobi(cells,cells.size());demand::jacobi(cells,cells.size());out.cells.push_back(std::move(cells));
 }
 // Extract over whole sequences, preserving candidate windows across learned seams.
 out.objects=discovery::extract(sources,4,out.costs);if(out.objects.empty()||out.objects.size()>16)throw std::invalid_argument("composition requires 1..16 extracted objects; no silent truncation");
 discovery::Directory directory(out.objects);auto candidates=discovery::tiled_pairs(directory,256);out.candidates=candidates.counts;
 std::vector<tensor::Window> windows;for(const auto& object:out.objects){auto exact=revisit(sources,object);auto map=object.source;map.origin=std::min(object.first,object.last);map.length=object.exact.size();windows.push_back({object.id,PackedFixture(exact.exact),map});}
 auto cohort=tensor::pack_windows(windows);ce_moon::tensor::Matrix16 q{},k{};unsigned rows=out.objects.size();for(unsigned i=0;i<rows;++i){q[i*16]=cohort.features[i*16+2];q[i*16+1]=cohort.features[i*16+1];k[i*16]=cohort.features[i*16];k[i*16+1]=cohort.features[i*16+3];}
 ce_moon::tensor::PairMask mask{};for(auto pair:candidates.records){const auto& from=out.objects.at(pair.from);const auto& to=out.objects.at(pair.to);if(owner(sources,from)!=owner(sources,to)&&from.source.strand==to.source.strand)mask[pair.from*16+pair.to]=1;}
 auto scores=ce_moon::tensor::relation_scores(q,k,rows,2);auto selected=ce_moon::tensor::compact_relations(scores,cohort.ids,rows,mask,0,pair_capacity);out.required=selected.required;out.overflow=selected.overflow;
 for(auto pair:selected.pairs){const auto& from=out.objects.at(pair.row);const auto& to=out.objects.at(pair.column);Edge edge{from.id,to.id,pair.score,owner(sources,from),owner(sources,to),revisit(sources,from),revisit(sources,to),{}};std::set<std::size_t> touched;
  for(std::size_t p=to.local;p<to.local+to.exact.size();++p)touched.insert(out.inverse[edge.to_source].at(p));
  for(auto chunk:touched){auto& cells=out.cells[edge.to_source];demand::wake(cells,{chunk,from.source.source_id,1,cells[chunk].context_version});edge.destination_chunks.push_back(chunk);}out.edges.push_back(std::move(edge));
 }
 for(unsigned wave=0;wave<wave_budget;++wave){bool active=false;for(const auto& cells:out.cells)for(const auto& cell:cells)active|=cell.active;if(!active){out.converged=true;break;}
  ++out.waves;for(auto& cells:out.cells){auto counts=demand::jacobi(cells,wave_capacity);out.updates+=counts.updated;out.deferred_updates+=counts.deferred;}}
 if(!out.converged){out.converged=true;for(const auto& cells:out.cells)for(const auto& cell:cells)out.converged&=!cell.active;}
 return out;
}
} // namespace bp_moon::compositions::nonlocal
