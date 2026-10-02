#pragma once
#include <bp_moon/reference.hpp>
#include <learning.hpp>
#include <memory>
#include <random>

namespace bp_moon::hypotheses {
enum class Strand { forward,reverse };
inline void validate_local_size(std::size_t n) {
    if(n>0x7fffffffu)throw std::invalid_argument("sequence fixture exceeds bounded local domain");
}
inline std::string bounded_sequence(std::string text) {
    validate_local_size(text.size());return text;
}
struct Source {
    u64 contig,origin,epoch;
    Strand strand;
    PackedFixture packed;
    Source(u64 c,u64 o,u64 e,Strand s,std::string sequence):contig(c),origin(o),epoch(e),strand(s),packed(bounded_sequence(std::move(sequence))) {
        if(packed.original.size()>std::numeric_limits<u64>::max()-origin)throw std::overflow_error("source coordinates");
        if(strand!=Strand::forward&&strand!=Strand::reverse)throw std::invalid_argument("source strand");
    }
};
using SourceRef=std::shared_ptr<const Source>;
struct Support {u64 contig,begin,end,epoch;Strand strand;};
inline Support support(const SourceRef& source,std::size_t begin,std::size_t end) {
    if(!source||end<begin||end>source->packed.original.size())throw std::invalid_argument("source support range");
    return {source->contig,source->origin+begin,source->origin+end,source->epoch,source->strand};
}
inline int canonical(char c) {
    switch(c){case 'A':case 'a':return 0;case 'C':case 'c':return 1;case 'G':case 'g':return 2;case 'T':case 't':return 3;default:return -1;}
}

// Persistent parse nodes share exact source and common prefixes. A branch is an
// interpretation of existing bases; it cannot rewrite the immutable source.
struct ParseNode {
    SourceRef source;
    std::shared_ptr<const ParseNode> parent;
    std::size_t begin,end;
    Support exact_support;
};
using Parse=std::shared_ptr<const ParseNode>;
inline Parse extend(const SourceRef& source,Parse parent,std::size_t end) {
    if(parent&&parent->source!=source)throw std::invalid_argument("parse source identity mismatch");
    std::size_t begin=parent?parent->end:0;
    if(end<=begin)throw std::invalid_argument("empty or reversed parse segment");
    return std::make_shared<const ParseNode>(ParseNode{source,parent,begin,end,support(source,begin,end)});
}
inline std::vector<Support> parse_supports(Parse branch) {
    std::vector<Support> out;for(auto node=branch;node;node=node->parent)out.push_back(node->exact_support);
    std::reverse(out.begin(),out.end());return out;
}
inline double branch_score(Parse branch,const std::array<double,2>& fitted_weights) {
    if(!branch||!branch->parent||branch->end!=branch->source->packed.original.size())
        throw std::invalid_argument("complete multi-segment branch required");
    const double row=double(branch->parent->end)/double(branch->end);
    return ce_moon::learning::predict_logistic(&row,1,fitted_weights.data());
}
struct Beam {std::vector<Parse> retained;std::size_t dropped;bool approximate;};
inline Beam retain_beam(std::vector<Parse> branches,const std::array<double,2>& weights,std::size_t capacity) {
    if(!capacity)throw std::invalid_argument("zero beam capacity");
    for(const auto& branch:branches){
        branch_score(branch,weights);
        if(branch->source!=branches.front()->source)throw std::invalid_argument("beam mixes different source identities");
    }
    std::stable_sort(branches.begin(),branches.end(),[&](Parse a,Parse b){return branch_score(a,weights)>branch_score(b,weights);});
    std::size_t dropped=branches.size()>capacity?branches.size()-capacity:0;
    if(dropped)branches.resize(capacity);
    return {std::move(branches),dropped,dropped!=0};
}

// Deliberately small eight-bit role-bound sketch. It nominates candidates only.
inline unsigned rotate8(unsigned value,unsigned position) {
    position%=8;return ((value<<position)|(value>>((8-position)%8)))&255u;
}
inline unsigned role_sketch(const SourceRef& source) {
    if(!source)throw std::invalid_argument("null sketch source");
    constexpr unsigned codes[5]{0x15,0x37,0x59,0x7b,0xe3};
    constexpr unsigned roles[4]{0x01,0x23,0x45,0x67};
    unsigned sketch=0;
    for(std::size_t i=0;i<source->packed.original.size();++i){int code=canonical(source->packed.original[i]);
        sketch^=rotate8(codes[code<0?4:code]^roles[i%4],unsigned(i%8));}
    return sketch;
}
struct SketchDecision {bool nominated,exact_equal;std::size_t replayed_bases;Support left,right;};
inline SketchDecision verify_sketch(const SourceRef& left,const SourceRef& right,unsigned hamming_limit=0) {
    if(hamming_limit>8)throw std::invalid_argument("sketch distance");
    bool nominated=popcount(role_sketch(left)^role_sketch(right))<=hamming_limit;
    bool same=false;std::size_t replayed=0;
    if(nominated){same=left->packed.original==right->packed.original;replayed=left->packed.original.size()+right->packed.original.size();}
    return {nominated,same,replayed,support(left,0,left->packed.original.size()),support(right,0,right->packed.original.size())};
}

struct Portfolio {
    SourceRef source;
    std::array<u64,4> counts{};
    u64 invalid=0;
    ce_moon::Dfa32 effect=ce_moon::Dfa32::identity();
    explicit Portfolio(SourceRef s):source(std::move(s)) {
        if(!source)throw std::invalid_argument("null portfolio source");
        for(char c:source->packed.original){int base=canonical(c);if(base<0)++invalid;else ++counts[base];}
        effect=summarize_sequence(source->packed.original);
    }
    Support exact_support() const {return support(source,0,source->packed.original.size());}
};
enum class Query { adenine_count,terminal_state,exact_motif };
enum class Route { count,effect,source_replay };
struct Answer {u64 value;Route route;std::size_t replayed_bases;Support exact_support;};
inline Answer ask(const Portfolio& p,Query query,std::string motif={}) {
    if(query==Query::adenine_count)return {p.counts[0],Route::count,0,p.exact_support()};
    if(query==Query::terminal_state)return {p.effect.to[0],Route::effect,0,p.exact_support()};
    if(query!=Query::exact_motif||motif.empty())throw std::invalid_argument("unsupported query");
    for(char c:motif)if(canonical(c)<0)throw std::invalid_argument("invalid motif base");
    u64 matches=0;
    const auto& text=p.source->packed.original;
    if(motif.size()<=text.size())for(std::size_t i=0;i<=text.size()-motif.size();++i){
        bool same=true;for(std::size_t j=0;j<motif.size();++j)
            if(!p.source->packed.is_valid(i+j)||canonical(text[i+j])!=canonical(motif[j])){same=false;break;}
        if(same)++matches;
    }
    return {matches,Route::source_replay,text.size(),p.exact_support()};
}

// Repair creates a new interpretation index, leaving previous snapshots intact.
// Only demonstrated histogram failures acquire a residual terminal-state key.
struct RepairIndex {
    std::vector<Portfolio> sources;
    std::map<std::size_t,unsigned> residual;
    struct RepairReceipt {std::size_t sites;bool failed_question_separated;};
    explicit RepairIndex(std::vector<Portfolio> inputs):sources(std::move(inputs)){}
    RepairIndex repaired(std::size_t a,std::size_t b,RepairReceipt* receipt=nullptr) const {
        if(a>=sources.size()||b>=sources.size())throw std::out_of_range("repair site");
        bool same=sources[a].counts==sources[b].counts&&sources[a].invalid==sources[b].invalid;
        bool failure=same&&sources[a].effect.to[0]!=sources[b].effect.to[0];
        RepairIndex next=*this;
        if(failure){next.residual[a]=sources[a].effect.to[0];next.residual[b]=sources[b].effect.to[0];}
        if(receipt)*receipt={failure?2u:0u,failure};
        return next;
    }
    Answer terminal(std::size_t id) const {
        if(id>=sources.size())throw std::out_of_range("repair query");
        auto it=residual.find(id);
        if(it!=residual.end())return {it->second,Route::effect,0,sources[id].exact_support()};
        auto state=summarize_sequence(sources[id].source->packed.original).to[0];
        return {state,Route::source_replay,sources[id].source->packed.original.size(),sources[id].exact_support()};
    }
    std::size_t exploration_site(unsigned seed) const {
        if(sources.empty())throw std::invalid_argument("empty exploration population");
        std::mt19937 rng(seed);return std::uniform_int_distribution<std::size_t>(0,sources.size()-1)(rng);
    }
};
} // namespace bp_moon::hypotheses
