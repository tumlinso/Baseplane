#pragma once
// Research seeds, not Baseplane's public ABI. No biological claims are implied.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <numeric>
#include <queue>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <ce_moon/reference.hpp>
namespace bp_moon {
using u32 = std::uint32_t;
using u64 = std::uint64_t;
inline unsigned popcount(u32 x) noexcept {
    unsigned n=0; for (;x;x&=x-1) ++n; return n;
}
inline u32 low_mask(unsigned n) {
    if (n>32) throw std::invalid_argument("mask width > 32");
    return n==32 ? ~u32{0} : ((u32{1}<<n)-1);
}
inline unsigned rank_before(u32 mask, unsigned lane) {
    if (lane>32) throw std::invalid_argument("invalid lane");
    return popcount(mask & low_mask(lane));
}
inline unsigned select_bit(u32 mask, unsigned rank) {
    if (rank>=popcount(mask)) throw std::out_of_range("rank outside support");
    while (rank--) mask &= mask-1;
    unsigned lane=0; while (!(mask & 1)) { ++lane; mask>>=1; } return lane;
}
// LUT index = 4*a + 2*b + c, matching NVIDIA lop3's truth-table convention.
inline u32 lut3(u32 a,u32 b,u32 c,unsigned lut) {
    if (lut>255) throw std::invalid_argument("LUT must contain eight bits");
    u32 out=0;
    for (unsigned k=0;k<8;++k) if ((lut>>k)&1)
        out |= ((k&4)?a:~a)&((k&2)?b:~b)&((k&1)?c:~c);
    return out;
}
struct CounterPlanes {
    std::array<u32,4> bits{};
    u32 overflow=0;
    void add(u32 support) noexcept {
        u32 carry=support;
        for (auto& b:bits) { u32 next=b&carry; b^=carry; carry=next; }
        overflow |= carry; // Explicit overflow: not a saturating counter.
    }
    unsigned at(unsigned lane) const {
        if (lane>=32) throw std::out_of_range("lane");
        unsigned v=0; for(unsigned b=0;b<4;++b)v|=((bits[b]>>lane)&1)<<b; return v;
    }
};
struct Planes { u32 lo=0,hi=0,valid=0; };
struct PackedFixture {
    // A deliberately small fixture representation; adapt to dna2_valid_view later.
    std::string original;
    std::vector<u64> words;
    std::vector<u32> valid;
    explicit PackedFixture(std::string s):original(std::move(s)),
        words((original.size()+31)/32),valid(words.size()) {
        for(std::size_t i=0;i<original.size();++i){
            unsigned c=0; bool ok=true;
            switch(original[i]){
                case 'A':case 'a':c=0;break;case 'C':case 'c':c=1;break;
                case 'G':case 'g':c=2;break;case 'T':case 't':c=3;break;
                default:ok=false;
            }
            words[i/32] |= u64(c)<<(2*(i%32));
            if(ok)valid[i/32]|=u32{1}<<(i%32);
        }
    }
    Planes planes(std::size_t w) const {
        if(w>=words.size())throw std::out_of_range("word");
        Planes p{0,0,valid[w]};
        for(unsigned i=0;i<32;++i){
            unsigned c=unsigned((words[w]>>(2*i))&3);
            p.lo |= u32(c&1)<<i; p.hi |= u32(c>>1)<<i;
        } return p;
    }
    bool is_valid(std::size_t i) const {
        if(i>=original.size())return false; return (valid[i/32]>>(i%32))&1;
    }
};
// A toy finite-state sequence mechanism: remember the last two canonical bases.
// Invalid bases reset state. This is NOT a learned biological interpretation.
inline ce_moon::Dfa32 base_transition(char b){
    int code=-1;
    switch(b){case 'A':case 'a':code=0;break;case 'C':case 'c':code=1;break;case 'G':case 'g':code=2;break;case 'T':case 't':code=3;break;}
    ce_moon::Dfa32 t;for(unsigned s=0;s<32;++s)t.to[s]=code<0?0u:((s*4+unsigned(code))&15u);return t;
}
inline ce_moon::Dfa32 summarize_sequence(const std::string& s){
    auto t=ce_moon::Dfa32::identity();for(char c:s)t=ce_moon::compose(t,base_transition(c));return t;
}
struct Interval{u64 contig,begin,end;};
inline std::vector<Interval> canonical_support(std::vector<Interval> x){
    for(auto s:x)if(s.end<s.begin)throw std::invalid_argument("inverted interval");
    std::sort(x.begin(),x.end(),[](auto a,auto b){return std::tie(a.contig,a.begin,a.end)<std::tie(b.contig,b.begin,b.end);});
    std::vector<Interval> out;for(auto s:x){if(s.begin==s.end)continue;
        if(!out.empty()&&out.back().contig==s.contig&&s.begin<=out.back().end)out.back().end=std::max(out.back().end,s.end);else out.push_back(s);}
    return out; // Never invent support in a gap merely by taking a bounding box.
}
struct ExactInterner {
    std::map<std::string,u64> dictionary;
    u64 intern(const std::string& exact){auto p=dictionary.find(exact);if(p!=dictionary.end())return p->second;
        const u64 id=dictionary.size();dictionary.emplace(exact,id);return id;}
};
} // namespace bp_moon
