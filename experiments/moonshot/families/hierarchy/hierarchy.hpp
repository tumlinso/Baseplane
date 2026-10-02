#pragma once
#include <bp_moon/source.hpp>
#include <set>

namespace bp_moon::hierarchy {
struct Span { std::size_t begin, end; };
inline bool gc(char c) { return c=='C'||c=='c'||c=='G'||c=='g'; }
inline void check_source(const PackedFixture& s,const SourceMap& m) {
    if(s.original.size()!=m.length)throw std::invalid_argument("source length");
    if(m.length){m.coordinate(0);m.coordinate(m.length-1);}
}
inline std::vector<u64> support(const PackedFixture& s,const SourceMap& m,Span p) {
    check_source(s,m);
    if(p.begin>p.end||p.end>m.length)throw std::out_of_range("span");
    std::vector<u64> out;
    for(auto i=p.begin;i<p.end;++i)if(s.is_valid(i))out.push_back(m.coordinate(i));
    return out;
}
// E13: valid sequence positions supply signed GC/AT carriers; invalid payloads
// remain in the source but are absent from the numerical carrier support.
struct Reservoir {
    SourceMap source;
    std::vector<std::size_t> positions;
    std::vector<double> coarse,residual;
    bool odd_tail=false;
    double tail=0;
    Reservoir(const PackedFixture& s,SourceMap m):source(m) {
        check_source(s,m);std::vector<double> x;
        for(std::size_t i=0;i<m.length;++i)if(s.is_valid(i)){
            positions.push_back(i);x.push_back(gc(s.original[i])?1.:-1.);
        }
        for(std::size_t i=0;i+1<x.size();i+=2){auto pair=ce_moon::lift(x[i],x[i+1]);coarse.push_back(pair.coarse);residual.push_back(pair.detail);}
        odd_tail=x.size()%2;if(odd_tail)tail=x.back();
    }
    std::vector<double> expand(bool details) const {
        std::vector<double> out;for(std::size_t i=0;i<coarse.size();++i){
            auto x=ce_moon::unlift({coarse[i],details?residual[i]:0.});out.push_back(x.first);out.push_back(x.second);}
        if(odd_tail)out.push_back(tail);return out;
    }
    std::size_t residual_bytes() const {return residual.size()*sizeof(double);}
    std::vector<double> replay(const PackedFixture& s,const SourceMap& current) const {
        if(current.source_id!=source.source_id||current.version!=source.version||
           current.contig!=source.contig||current.origin!=source.origin||
           current.strand!=source.strand||current.length!=source.length)
            throw std::invalid_argument("stale source replay");
        check_source(s,current);std::vector<double> out;
        for(auto p:positions){if(!s.is_valid(p))throw std::invalid_argument("changed validity");
            out.push_back(gc(s.original[p])?1.:-1.);}
        return out;
    }
};
inline std::vector<Span> chunks(std::size_t n,const std::vector<std::size_t>& cuts) {
    std::vector<Span> out;std::size_t a=0;
    for(auto z:cuts){if(z<=a||z>=n)throw std::invalid_argument("noncanonical cut");out.push_back({a,z});a=z;}
    if(a<n)out.push_back({a,n});return out;
}
inline std::vector<std::size_t> inverse_map(std::size_t n,const std::vector<Span>& spans){
    std::vector<std::size_t> out(n);std::size_t expected=0;
    for(std::size_t j=0;j<spans.size();++j){auto p=spans[j];if(p.begin!=expected||p.end<=p.begin||p.end>n)throw std::invalid_argument("chunk gap");
        for(auto i=p.begin;i<p.end;++i)out[i]=j;expected=p.end;}
    if(expected!=n)throw std::invalid_argument("incomplete chunks");return out;
}
// E15: ten fractional GC-count planes over a bounded 1024-base span. The
// denominator is fixed and explicit, so the integer count is reconstructed
// exactly at the last plane; this does not certify arbitrary learned latents.
struct PrecisionQuery { bool above=false; unsigned planes=0; double lower=0,upper=0; };
inline PrecisionQuery precision_query(const PackedFixture& s,Span p,double threshold){
    if(p.begin>p.end||p.end>s.original.size()||p.end-p.begin>1024||!std::isfinite(threshold))throw std::invalid_argument("precision bounds");
    unsigned count=0;for(auto i=p.begin;i<p.end;++i)count+=s.is_valid(i)&&gc(s.original[i]);
    PrecisionQuery r;
    // Eleven planes cover the inclusive count 1024.
    for(unsigned bits=1;bits<=11;++bits){const unsigned shift=11-bits;
        const unsigned low=(count>>shift)<<shift;
        const unsigned high=std::min(1024u,low+((1u<<shift)-1));
        r={false,bits,low/1024.,high/1024.};
        if(r.lower>threshold){r.above=true;return r;}
        if(r.upper<=threshold)return r;
    }
    throw std::logic_error("exact plane remains ambiguous");
}
// E16 uses a sliding content window, not the prefix history, so fingerprints
// converge after an insertion leaves that window. Length caps are mandatory.
inline std::vector<Span> content_chunks(const PackedFixture& s,std::size_t min_len,
                                      std::size_t max_len,unsigned offset=0){
    if(!min_len||max_len<min_len)throw std::invalid_argument("chunk caps");
    std::vector<std::size_t> cuts;std::size_t begin=0;
    for(std::size_t end=1;end<s.original.size();++end){
        std::uint64_t h=1469598103934665603ULL;
        for(auto i=end>4?end-4:0;i<end;++i){h^=static_cast<unsigned char>(s.original[i]);h*=1099511628211ULL;}
        if(end-begin>=max_len||(end-begin>=min_len&&((h+offset)&3u)==0)){cuts.push_back(end);begin=end;}
    }
    return chunks(s.original.size(),cuts);
}
struct SeamResult { std::vector<std::size_t> positions;std::size_t replayed=0,alternative=0; };
inline bool contains(const std::vector<Span>& forest,std::size_t p,std::size_t len){
    for(auto span:forest)if(span.begin<=p&&p<span.end&&len<=span.end-p)return true;return false;
}
inline SeamResult motif_query(const PackedFixture& s,const std::string& motif,
                             const std::vector<Span>& first,const std::vector<Span>& second){
    if(motif.empty())throw std::invalid_argument("empty motif");
    inverse_map(s.original.size(),first);inverse_map(s.original.size(),second);
    SeamResult out;
    for(std::size_t p=0;p<=s.original.size()&&motif.size()<=s.original.size()-p;++p){
        if(!contains(first,p,motif.size())){if(contains(second,p,motif.size()))++out.alternative;else ++out.replayed;}
        bool match=true;for(std::size_t j=0;j<motif.size();++j)match&=s.is_valid(p+j)&&s.original[p+j]==motif[j];
        if(match)out.positions.push_back(p);
    }return out;
}
inline std::size_t reused_chunks(const PackedFixture& a,const std::vector<Span>& ca,
                                const PackedFixture& b,const std::vector<Span>& cb){
    std::set<std::string> exact;for(auto p:ca)exact.insert(a.original.substr(p.begin,p.end-p.begin));
    std::size_t reused=0;for(auto p:cb)reused+=exact.count(b.original.substr(p.begin,p.end-p.begin));return reused;
}
} // namespace bp_moon::hierarchy
