#pragma once
#include <bp_moon/source.hpp>
#include <set>
#include "e18_layout.hpp"
namespace bp_moon::discovery {
using ce_moon::checked_add;
struct Sequence { PackedFixture sequence; SourceMap source; };
struct Object { u64 id; SourceMap source; std::size_t local; u64 first,last; std::string exact; };
struct Costs { u64 inspected=0, objects=0, probes=0, collisions=0, verifications=0; };
inline char canonical(char x) {
    switch(x) {case 'a':case 'A':return 'A';case 'c':case 'C':return 'C';
    case 'g':case 'G':return 'G';case 't':case 'T':return 'T';default:return 'N';}
}
// No caller-supplied candidate keys: every valid sliding window is visited.
inline std::vector<Object> extract(const std::vector<Sequence>& input, std::size_t width, Costs& cost) {
    if(!width) throw std::invalid_argument("zero window");
    std::vector<Object> out;
    for(const auto& s:input) {
        if(s.source.length!=s.sequence.original.size()) throw std::invalid_argument("source length");
        if(width>s.source.length) continue;
        for(std::size_t p=0;p<=s.source.length-width;++p) {
            std::string exact; bool valid=true;
            for(std::size_t j=0;j<width;++j) {
                cost.inspected=checked_add(cost.inspected,1);
                valid &= s.sequence.is_valid(p+j);
                exact.push_back(canonical(s.sequence.original[p+j]));
            }
            if(valid) out.push_back({u64(out.size()),s.source,p,s.source.coordinate(p),s.source.coordinate(p+width-1),std::move(exact)});
        }
    }
    cost.objects=out.size();return out;
}
struct Group { std::string key; std::vector<u64> ids; };
struct Directory {
    std::vector<Group> groups;
    explicit Directory(const std::vector<Object>& objects) {
        std::vector<std::pair<std::string,u64>> records;
        for(const auto& o:objects) records.emplace_back(o.exact,o.id);
        std::sort(records.begin(),records.end());
        for(const auto& r:records) {
            if(groups.empty()||groups.back().key!=r.first)groups.push_back({r.first,{}});
            groups.back().ids.push_back(r.second);
        }
    }
};
struct Pair {u64 from,to;};
struct PairOutput {std::vector<Pair> records; EmitCounts counts;};
inline void account(EmitCounts& c,bool stored) {
    c.produced=checked_add(c.produced,1);
    if(stored)c.stored=checked_add(c.stored,1);else c.dropped=checked_add(c.dropped,1);
}
inline PairOutput tiled_pairs(const Directory& d,std::size_t capacity,std::size_t tile=32) {
    if(!tile)throw std::invalid_argument("zero tile");
    PairOutput out;
    for(const auto& g:d.groups) {
        const u64 k=g.ids.size();
        if(k>1 && k>std::numeric_limits<u64>::max()/(k-1))throw std::overflow_error("pair count");
        // Count all pairs algebraically; do not do quadratic work after capacity is exhausted.
        const u64 total=k*(k?k-1:0);
        out.counts.produced=checked_add(out.counts.produced,total);
        for(std::size_t a=0;a<g.ids.size() && out.records.size()<capacity;++a)
            for(std::size_t begin=0;begin<g.ids.size() && out.records.size()<capacity;begin+=std::min(tile,g.ids.size()-begin))
                for(std::size_t b=begin;b<begin+std::min(tile,g.ids.size()-begin) && out.records.size()<capacity;++b)
                    if(a!=b)out.records.push_back({g.ids[a],g.ids[b]});
    }
    out.counts.stored=out.records.size();out.counts.dropped=out.counts.produced-out.counts.stored;return out;
}
inline u64 hash(const std::string& key) {u64 h=14695981039346656037ull;for(unsigned char c:key){h^=c;h*=1099511628211ull;}return h;}
// Immutable bulk-built rows. Full string equality protects against hash collisions.
struct PostingHash {
    struct Row { std::vector<std::size_t> groups; std::size_t next=std::numeric_limits<std::size_t>::max(); };
    const Directory& directory; std::vector<Row> rows;std::size_t buckets,width;u64 overflow=0;
    PostingHash(const Directory& d,std::size_t bucket_count,std::size_t row_width,Costs& cost):directory(d),buckets(bucket_count),width(row_width) {
        if(!buckets||!width||width>32)throw std::invalid_argument("bucket geometry");
        rows.resize(buckets);
        for(std::size_t g=0;g<d.groups.size();++g) {
            auto row=std::size_t(hash(d.groups[g].key)%buckets);
            for(;;) {
                cost.probes=checked_add(cost.probes,1);
                if(rows[row].groups.size()<width){rows[row].groups.push_back(g);break;}
                cost.collisions=checked_add(cost.collisions,1);
                if(rows[row].next==std::numeric_limits<std::size_t>::max()) {
                    const auto next=rows.size();rows[row].next=next;rows.emplace_back();++overflow;
                }
                row=rows[row].next;
            }
        }
    }
    const Group* find(const std::string& key,Costs& cost)const {
        auto row=std::size_t(hash(key)%buckets);
        while(row!=std::numeric_limits<std::size_t>::max()) {
            for(auto g:rows[row].groups){cost.probes=checked_add(cost.probes,1);if(directory.groups[g].key==key)return &directory.groups[g];}
            row=rows[row].next;
        }return nullptr;
    }
};
inline gpu::Query packed_key(const std::string& exact,unsigned bucket) {
    if(exact.empty()||exact.size()>32)throw std::invalid_argument("GPU key width must be 1..32");
    u64 key=0;for(char c:exact){unsigned code=0;switch(c){case 'A':break;case 'C':code=1;break;case 'G':code=2;break;case 'T':code=3;break;default:throw std::invalid_argument("noncanonical GPU key");}key=(key<<2)|code;}
    return {key,unsigned(exact.size()),bucket};
}
inline std::vector<gpu::Row> packed_rows(const PostingHash& table) {
    if(table.rows.size()>=gpu::absent||table.directory.groups.size()>=gpu::absent)throw std::overflow_error("GPU directory index");
    std::vector<gpu::Row> out(table.rows.size());
    for(std::size_t r=0;r<out.size();++r) {
        auto& row=out[r];for(auto& slot:row.slots)slot={0,0,gpu::absent};
        row.next=table.rows[r].next==std::numeric_limits<std::size_t>::max()?gpu::absent:unsigned(table.rows[r].next);
        for(std::size_t lane=0;lane<table.rows[r].groups.size();++lane){auto g=table.rows[r].groups[lane];auto key=packed_key(table.directory.groups[g].key,0);row.slots[lane]={key.key,key.width,unsigned(g)};}
    }return out;
}
struct Nomination { std::set<std::pair<u64,u64>> candidates, verified; u64 truth=0,missed=0,evaluation_comparisons=0; };
inline std::size_t distance(const std::string& a,const std::string& b) {
    if(a.size()!=b.size())throw std::invalid_argument("different widths");
    std::size_t d=0;for(std::size_t i=0;i<a.size();++i)d+=a[i]!=b[i];return d;
}
inline Nomination nominate(const std::vector<Object>& objects,unsigned probes,std::size_t tolerance,Costs& costs) {
    if(objects.size()>4096)throw std::length_error("nomination fixture limit 4096; larger runs need bounded streaming candidates");
    for(std::size_t i=0;i<objects.size();++i)if(objects[i].id!=i)throw std::invalid_argument("object id/index mismatch");
    if(probes<1||probes>2)throw std::invalid_argument("probe count");
    Nomination out;
    // Independent half-sequence sketches nominate; exact strings remain available for verification.
    for(unsigned probe=0;probe<probes;++probe) {
        std::map<std::string,std::vector<u64>> postings;
        for(const auto& o:objects) {
            const auto half=o.exact.size()/2;
            postings[probe==0?o.exact.substr(0,half):o.exact.substr(half)].push_back(o.id);
            costs.probes=checked_add(costs.probes,1);
        }
        for(const auto& p:postings)for(std::size_t a=0;a<p.second.size();++a)for(std::size_t b=a+1;b<p.second.size();++b)
            out.candidates.emplace(p.second[a],p.second[b]);
    }
    for(auto p:out.candidates) {costs.verifications=checked_add(costs.verifications,1);if(distance(objects.at(p.first).exact,objects.at(p.second).exact)<=tolerance)out.verified.insert(p);}
    // Small-fixture recall oracle used solely for evaluation, never for nomination.
    for(std::size_t a=0;a<objects.size();++a)for(std::size_t b=a+1;b<objects.size();++b)
        {++out.evaluation_comparisons;if(distance(objects[a].exact,objects[b].exact)<=tolerance){++out.truth;if(!out.candidates.count({objects[a].id,objects[b].id}))++out.missed;}}
    return out;
}
struct Request {std::string motif;u64 destination,source_id,contig,begin,end;Strand strand;};
struct Answer {u64 destination,source_id,contig,version,first,last;Strand strand;};
struct Inspection {u64 destination,source_id,contig,version,begin,end;Strand strand;};
struct Answers {std::vector<Answer> records;std::vector<Inspection> support;EmitCounts counts;u64 evaluations=0;};
inline Answers subscribe(const std::vector<Sequence>& sources,const std::vector<Request>& requests,std::size_t capacity) {
    std::map<std::string,std::vector<std::size_t>> cohorts;
    for(std::size_t i=0;i<requests.size();++i){const auto& r=requests[i];if(r.motif.empty()||r.end<r.begin)throw std::invalid_argument("subscription");std::string motif;for(char c:r.motif){c=canonical(c);if(c=='N')throw std::invalid_argument("invalid motif");motif+=c;}cohorts[motif].push_back(i);}
    Answers out;
    for(const auto& cohort:cohorts)for(const auto& s:sources) {
        if(s.source.length!=s.sequence.original.size())throw std::invalid_argument("source length");
        const auto width=cohort.first.size();if(width>s.source.length)continue;
        for(std::size_t p=0;p<=s.source.length-width;++p) {
            const auto first=s.source.coordinate(p),last=s.source.coordinate(p+width-1);
            const auto lo=std::min(first,last),hi=checked_add(std::max(first,last),1);
            std::vector<std::size_t> destinations;
            for(auto i:cohort.second){const auto& r=requests[i];if(r.source_id==s.source.source_id&&r.contig==s.source.contig&&r.strand==s.source.strand&&lo>=r.begin&&hi<=r.end)destinations.push_back(i);}
            if(destinations.empty())continue;
            out.evaluations=checked_add(out.evaluations,1);bool match=true;
            for(std::size_t j=0;j<width;++j)match &= s.sequence.is_valid(p+j)&&canonical(s.sequence.original[p+j])==cohort.first[j];
            for(auto i:destinations) {
                const auto& r=requests[i];out.support.push_back({r.destination,s.source.source_id,s.source.contig,s.source.version,lo,hi,s.source.strand});
                if(match){const bool store=out.records.size()<capacity;account(out.counts,store);if(store)out.records.push_back({r.destination,s.source.source_id,s.source.contig,s.source.version,first,last,s.source.strand});}
            }
        }
    }return out;
}
} // namespace bp_moon::discovery
