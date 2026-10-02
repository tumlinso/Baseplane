#pragma once
#include <bp_moon/source.hpp>
#include <set>

namespace bp_moon::incremental {
struct Span {std::size_t begin,end;};
inline void validate_source(const PackedFixture& s,const SourceMap& m){
    if(m.length!=s.original.size())throw std::invalid_argument("source length");
    if(m.length){m.coordinate(0);m.coordinate(m.length-1);}
}
inline bool same_source_identity(const SourceMap& a,const SourceMap& b){
    return std::tie(a.source_id,a.contig,a.origin,a.strand)==std::tie(b.source_id,b.contig,b.origin,b.strand);
}
inline bool is_gc(char c){return c=='G'||c=='g'||c=='C'||c=='c';}
inline unsigned gc_count(const PackedFixture& s,Span p){
    if(p.begin>p.end||p.end>s.original.size())throw std::out_of_range("query span");
    unsigned n=0;for(auto i=p.begin;i<p.end;++i)n+=s.is_valid(i)&&is_gc(s.original[i]);return n;
}
// E33: nominations may all collide. Equality is decided only by exact original
// tokens, including invalid symbols, rather than by the packed payload.
class RepeatQuotient {
public:
    struct Content {std::string exact;ce_moon::Dfa32 effect;unsigned gc;};
    struct Occurrence {std::size_t content;SourceMap source;u64 state_version;double residual;};
    std::vector<Content> contents;
    std::vector<Occurrence> occurrences;
    std::map<u64,std::vector<std::size_t>> buckets;
    std::size_t exact_comparisons=0;
    std::size_t add(const PackedFixture& s,SourceMap source,u64 state,double residual,u64 nomination){
        validate_source(s,source);if(!std::isfinite(residual))throw std::invalid_argument("context residual");
        auto& candidates=buckets[nomination];std::size_t id=contents.size();
        for(auto candidate:candidates){++exact_comparisons;if(contents[candidate].exact==s.original){id=candidate;break;}}
        if(id==contents.size()){contents.push_back({s.original,summarize_sequence(s.original),gc_count(s,{0,s.original.size()})});candidates.push_back(id);}
        occurrences.push_back({id,source,state,residual});return occurrences.size()-1;
    }
    double contextual_answer(std::size_t occurrence)const{
        const auto& o=occurrences.at(occurrence);return contents.at(o.content).gc+o.residual;
    }
};
// E34: source effect and structural posting membership are distinct dependency
// nodes. Directory keys here are exact strings, so hash collisions cannot alias.
struct DirtyCone {
    std::vector<std::vector<unsigned>> parents;
    std::map<std::string,std::set<unsigned>> directory;
    std::vector<std::string> keys;std::vector<SourceMap> sources;
    explicit DirtyCone(std::vector<std::vector<unsigned>> p,std::vector<std::string> k,std::vector<SourceMap> m):parents(std::move(p)),keys(std::move(k)),sources(std::move(m)){
        if(sources.size()!=keys.size())throw std::invalid_argument("directory source descriptors");
        for(unsigned i=0;i<keys.size();++i){validate_source(PackedFixture(keys[i]),sources[i]);directory[keys[i]].insert(i);}
    }
    std::vector<unsigned> replace(unsigned leaf,unsigned membership,const std::string& next,SourceMap current){
        if(leaf>=keys.size()||leaf>=parents.size()||membership>=parents.size())throw std::out_of_range("dirty source");
        if(!same_source_identity(sources[leaf],current)||current.version<=sources[leaf].version||current.version-sources[leaf].version!=1)throw std::invalid_argument("directory source version");
        validate_source(PackedFixture(next),current);
        std::vector<bool> seen(parents.size());std::vector<unsigned> frontier{leaf,membership};
        while(!frontier.empty()){auto n=frontier.back();frontier.pop_back();
            if(n>=parents.size())throw std::out_of_range("reverse dependency");
            if(seen[n])continue;seen[n]=true;for(auto parent:parents[n])frontier.push_back(parent);}
        std::vector<unsigned> dirty;for(unsigned n=0;n<seen.size();++n)if(seen[n])dirty.push_back(n);
        auto old=directory.find(keys[leaf]);if(old==directory.end()||!old->second.count(leaf))throw std::logic_error("missing membership");
        old->second.erase(leaf);if(old->second.empty())directory.erase(old);
        keys[leaf]=next;sources[leaf]=current;directory[next].insert(leaf);return dirty;
    }
};
struct QueryIdentity {
    u64 query,weights,cell_state,representation;
    bool operator==(const QueryIdentity& x)const{return std::tie(query,weights,cell_state,representation)==std::tie(x.query,x.weights,x.cell_state,x.representation);}
};
struct Edit {u64 from,to;std::size_t position,removed,inserted;};
// E35 records all inspected bases, including negative evidence. Reuse after a
// source-version change requires a complete ordered edit chain. Length changes
// at/before dependency support invalidate coordinate-bearing results.
struct Memo {
    SourceMap source;QueryIdentity identity;std::vector<Span> support;unsigned answer;
    static Memo count(const PackedFixture& s,SourceMap m,QueryIdentity q,Span span){
        validate_source(s,m);return {m,q,{span},gc_count(s,span)};
    }
    bool reusable(const SourceMap& current,QueryIdentity q,const std::vector<Edit>& edits)const{
        if(!same_source_identity(source,current)||!(identity==q)||current.version<source.version)return false;
        if(current.version==source.version)return edits.empty()&&current.length==source.length;
        auto version=source.version;std::size_t length=source.length;
        for(auto e:edits){
            if(e.from!=version||e.to<=e.from||e.to-e.from!=1||e.position>length||e.removed>length-e.position)return false;
            if(e.inserted>std::numeric_limits<std::size_t>::max()-(length-e.removed))return false;
            for(auto p:support){
                const bool resized=e.removed!=e.inserted;
                // Even support before a resize can receive new coordinates on a
                // reverse view whose origin anchors the full sequence extent.
                if(resized&&(source.strand==Strand::reverse||e.position<p.end))return false;
                if(e.position<p.end&&e.position+e.removed>p.begin)return false;
            }
            length=length-e.removed+e.inserted;version=e.to;
        }
        return version==current.version&&length==current.length;
    }
};
// A plan caches query shape only. It never returns a memoized numerical answer.
struct CountPlan {Span span;unsigned execute(const PackedFixture& s)const{return gc_count(s,span);}};
// E36: intern ordered child pairs at several scales and compose their DFA effects
// through Cellerator. Occurrence spans remain outside the shared grammar DAG.
class Grammar {
public:
    struct Node {bool leaf;char token;std::size_t left,right,length;ce_moon::Dfa32 effect;};
    struct Occurrence {std::size_t node;SourceMap source;Span span;};
    std::vector<Node> nodes;std::vector<Occurrence> occurrences;
    std::map<char,std::size_t> leaves;
    std::map<std::pair<std::size_t,std::size_t>,std::size_t> pairs;
    std::size_t compositions=0;
    std::size_t token(char c){auto found=leaves.find(c);if(found!=leaves.end())return found->second;
        auto id=nodes.size();nodes.push_back({true,c,0,0,1,base_transition(c)});leaves[c]=id;return id;}
    std::size_t pair(std::size_t left,std::size_t right){
        const auto key=std::make_pair(left,right);auto found=pairs.find(key);if(found!=pairs.end())return found->second;
        const auto& l=nodes.at(left);const auto& r=nodes.at(right);
        if(r.length>std::numeric_limits<std::size_t>::max()-l.length)throw std::overflow_error("phrase length");
        auto effect=ce_moon::compose(l.effect,r.effect);auto length=l.length+r.length;
        auto id=nodes.size();nodes.push_back({false,0,left,right,length,effect});pairs[key]=id;++compositions;return id;
    }
    std::size_t parse(const std::string& s,std::size_t begin,std::size_t end){
        if(begin>=end||end>s.size())throw std::invalid_argument("empty phrase");
        if(end-begin==1)return token(s[begin]);auto mid=begin+(end-begin)/2;
        const auto left=parse(s,begin,mid),right=parse(s,mid,end);return pair(left,right);
    }
    std::size_t add(const PackedFixture& s,SourceMap m,Span span){
        validate_source(s,m);auto id=parse(s.original,span.begin,span.end);occurrences.push_back({id,m,span});return id;
    }
    std::string expand(std::size_t id)const{const auto& n=nodes.at(id);if(n.leaf)return std::string(1,n.token);return expand(n.left)+expand(n.right);}
    std::vector<u64> coordinates(std::size_t occurrence)const{
        const auto& o=occurrences.at(occurrence);std::vector<u64> result;
        for(auto p=o.span.begin;p<o.span.end;++p)result.push_back(o.source.coordinate(p));return result;
    }
};
} // namespace bp_moon::incremental
