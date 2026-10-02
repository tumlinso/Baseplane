#pragma once
#include <bp_moon/source.hpp>

namespace bp_moon::logic {
// All mask operands belong to exactly the same source word and version.
struct Domain {
    u64 source, contig, version, word;
    bool operator==(const Domain& x) const {
        return source==x.source && contig==x.contig && version==x.version && word==x.word;
    }
};
struct Mask { Domain domain; u32 bits, valid; };
inline void compatible(const Mask& a,const Mask& b) {
    if (!(a.domain==b.domain) || a.valid!=b.valid) throw std::invalid_argument("mask domain");
}
struct Circuit {
    // This is a fixed fixture circuit, not a trained truth table.
    unsigned gate=0x96, output_gate=0x80;
    u64 parameter_version=1;
    Mask apply(const Mask& a,const Mask& b,const Mask& c) const {
        compatible(a,b); compatible(a,c);
        const auto hidden=lut3(a.bits,b.bits,c.bits,gate)&a.valid;
        return {a.domain,lut3(hidden,b.bits,c.bits,output_gate)&a.valid,a.valid};
    }
};
struct Evidence {
    CounterPlanes wrap;
    CounterPlanes saturated;
    u32 valid;
    explicit Evidence(u32 v):valid(v){}
    void add(u32 support) {
        support &= valid;
        wrap.add(support);
        const u32 full=saturated.bits[0]&saturated.bits[1]&saturated.bits[2]&saturated.bits[3];
        saturated.overflow |= full & support;
        saturated.add(support & ~full);
    }
    u32 at_least(unsigned threshold,bool saturating=false) const {
        if(threshold>15) throw std::invalid_argument("four-bit threshold");
        const auto& p=saturating?saturated:wrap;
        u32 greater=0,equal=valid;
        for(int b=3;b>=0;--b) {
            if((threshold>>b)&1) equal &= p.bits[b];
            else {greater |= equal&p.bits[b]; equal &= ~p.bits[b];}
        }
        return (greater|equal)&valid;
    }
};
inline char complement(char c) {
    switch(c) {case 'A':return 'T';case 'C':return 'G';case 'G':return 'C';case 'T':return 'A';}
    throw std::invalid_argument("canonical motif required");
}
inline u32 base_mask(const PackedFixture& s,std::size_t word,char b) {
    if(word>=s.words.size()) return 0;
    const auto p=s.planes(word);
    switch(b) {
        case 'A': return (~p.lo&~p.hi)&p.valid;
        case 'C': return (p.lo&~p.hi)&p.valid;
        case 'G': return (~p.lo&p.hi)&p.valid;
        case 'T': return (p.lo&p.hi)&p.valid;
    }
    throw std::invalid_argument("canonical motif required");
}
// Pull position + offset to its anchor. Adjacent word supplies the halo.
inline u32 funnel(u32 current,u32 next,unsigned offset) {
    if(offset>=32) throw std::invalid_argument("local grammar span");
    return offset ? (current>>offset)|(next<<(32-offset)) : current;
}
inline u32 motif_mask(const PackedFixture& s,std::size_t word,const std::string& motif) {
    if(motif.empty() || motif.size()>32) throw std::invalid_argument("motif length");
    u32 selected=s.planes(word).valid;
    for(unsigned j=0;j<motif.size();++j)
        selected &= funnel(base_mask(s,word,motif[j]),base_mask(s,word+1,motif[j]),j);
    return selected;
}
enum class Duplicates { both_strands, canonical_anchor };
struct Anchor { u64 source,contig,coordinate,version; Strand strand; };
inline std::vector<Anchor> grammar(const PackedFixture& s,const SourceMap& map,
                                   const std::string& motif,Duplicates policy) {
    if(map.length!=s.original.size() || map.strand!=Strand::forward)
        throw std::invalid_argument("grammar requires original forward coordinate map");
    std::string rc;for(auto it=motif.rbegin();it!=motif.rend();++it)rc+=complement(*it);
    std::vector<Anchor> out;
    for(std::size_t w=0;w<s.words.size();++w) {
        const u32 f=motif_mask(s,w,motif),r=motif_mask(s,w,rc);
        for(unsigned lane=0;lane<32;++lane) {
            const u32 bit=u32{1}<<lane;
            // A reverse hit's coordinate is its high original-coordinate anchor.
            if(f&bit)out.push_back({map.source_id,map.contig,map.coordinate(w*32+lane),map.version,Strand::forward});
            if((r&bit) && (policy==Duplicates::both_strands || !(f&bit)))
                out.push_back({map.source_id,map.contig,map.coordinate(w*32+lane+motif.size()-1),map.version,Strand::reverse});
        }
    }
    return out;
}
inline Mask intersect(const Mask& a,const Mask& b) {
    compatible(a,b);return {a.domain,a.bits&b.bits&a.valid,a.valid};
}
inline u32 compose_support(u32 parent,u32 child) {
    if(child&~low_mask(popcount(parent))) throw std::invalid_argument("child outside parent support");
    u32 out=0;
    for(unsigned rank=0;rank<popcount(parent);++rank)
        if(child&(u32{1}<<rank))out|=u32{1}<<select_bit(parent,rank);
    return out;
}
template<class T> inline std::vector<T> route(const Mask& parent,const Mask& query,const std::vector<T>& compact) {
    compatible(parent,query);
    const auto support=parent.bits&parent.valid;
    if(compact.size()!=popcount(support)) throw std::invalid_argument("descriptor count");
    const auto selected=intersect(parent,query).bits;
    std::vector<T> out;
    for(unsigned r=0;r<popcount(selected);++r) {
        const unsigned lane=select_bit(selected,r);
        out.push_back(compact[rank_before(support,lane)]);
    }
    return out;
}
} // namespace bp_moon::logic
