#pragma once
#include <bp_moon/source.hpp>
#include <cstring>
namespace bp_moon::machine {
struct Input {PackedFixture sequence;SourceMap source;};
inline void validate(const Input& in){if(in.source.length!=in.sequence.original.size())throw std::invalid_argument("source length");}
struct Features {std::array<std::int8_t,4> bytes{};float scale=.25f;int zero_point=0;};
// Four sequence base counts, centered at four. Invalid payload never contributes.
inline Features features(const Input& in,std::size_t word){validate(in);auto p=in.sequence.planes(word);Features f;std::array<unsigned,4> counts{};
    for(unsigned lane=0;lane<32;++lane)if((p.valid>>lane)&1)++counts[unsigned((in.sequence.words[word]>>(2*lane))&3)];
    for(unsigned b=0;b<4;++b)f.bytes[b]=std::int8_t(int(counts[b])-4);return f;}
inline int packed(const std::array<std::int8_t,4>& x){u32 p=0;for(unsigned i=0;i<4;++i)p|=u32(std::uint8_t(x[i]))<<(8*i);int out;static_assert(sizeof(out)==sizeof(p));std::memcpy(&out,&p,sizeof(p));return out;}
inline int scalar_dot(const Features& f,const std::array<std::int8_t,4>& weights){int sum=0;for(unsigned i=0;i<4;++i)sum+=int(f.bytes[i])*int(weights[i]);return sum;}
inline std::array<float,32> butterfly_reference(std::array<float,32> x){for(unsigned step=1;step<32;step<<=1){auto old=x;for(unsigned lane=0;lane<32;++lane)x[lane]=(lane&step)?old[lane^step]-old[lane]:old[lane]+old[lane^step];}return x;}
// Host texture oracle uses mathematical float interpolation, without emulating hardware quantization.
struct Surface {unsigned width,height;std::vector<float> table;};
inline float response(const Surface& t,float x,float y,bool linear){
    if(!t.width||!t.height||t.table.size()!=std::size_t(t.width)*t.height||!std::isfinite(x)||!std::isfinite(y))throw std::invalid_argument("surface");
    auto at=[&](int a,int b){a=std::max(0,std::min(int(t.width)-1,a));b=std::max(0,std::min(int(t.height)-1,b));return t.table[std::size_t(b)*t.width+unsigned(a)];};
    x=std::max(-1.f,std::min(float(t.width),x));y=std::max(-1.f,std::min(float(t.height),y));
    if(!linear)return at(int(std::floor(x+.5f)),int(std::floor(y+.5f)));
    int a=int(std::floor(x)),b=int(std::floor(y));float dx=x-a,dy=y-b;
    return (1-dy)*((1-dx)*at(a,b)+dx*at(a+1,b))+dy*((1-dx)*at(a,b+1)+dx*at(a+1,b+1));}
inline std::pair<float,float> coordinates(const Input& in,std::size_t word){auto f=features(in,word);unsigned n=popcount(in.sequence.valid.at(word));if(!n)return {0,0};return {3.f*float(int(f.bytes[1])+4)/n,3.f*float(int(f.bytes[2])+4)/n};}
inline u32 transpose_reference(u32 x){u32 y=0;for(unsigned r=0;r<4;++r)for(unsigned c=0;c<4;++c)y|=((x>>(4*r+c))&1)<<(4*c+r);return y;}
inline u32 transpose_network(u32 x,u32 first=0x0a0a,u32 second=0x00cc){u32 t=(x^(x>>3))&first;x^=t^(t<<3);t=(x^(x>>6))&second;x^=t^(t<<6);return x&0xffff;}
inline u32 spread_nibble(u32 x){return (x&1)|((x&2)<<3)|((x&4)<<6)|((x&8)<<9);}
inline u32 transpose_lookup(u32 x,unsigned spacing=4){std::array<u32,16> table{};for(unsigned i=0;i<16;++i)for(unsigned bit=0;bit<4;++bit)table[i]|=((i>>bit)&1)<<(spacing*bit);u32 out=0;for(unsigned row=0;row<4;++row)out|=table[(x>>(4*row))&15]<<row;return out;}
struct Search {unsigned tested=0,lookup_tested=0,lookup_survivors=0;std::vector<std::pair<u32,u32>> survivors;u64 exhaustive_checks=0;};
inline Search synthesize(){Search result;for(u32 first:{0x0a0au,0x0505u,0x0f0fu})for(u32 second:{0x00ccu,0x0033u}){++result.tested;bool valid=true;for(u32 x=0;x<65536;++x){++result.exhaustive_checks;if(transpose_network(x,first,second)!=transpose_reference(x)){valid=false;break;}}if(valid)result.survivors.emplace_back(first,second);}
    for(unsigned spacing:{1u,4u}){++result.lookup_tested;bool valid=true;for(u32 x=0;x<65536;++x){++result.exhaustive_checks;if(transpose_lookup(x,spacing)!=transpose_reference(x)){valid=false;break;}}if(valid)++result.lookup_survivors;}return result;}
inline unsigned gc(Planes p){return popcount((p.lo^p.hi)&p.valid);}
inline std::vector<unsigned> fused(const Input& in){validate(in);std::vector<unsigned> out;for(std::size_t w=0;w<in.sequence.words.size();++w)out.push_back(gc(in.sequence.planes(w)));return out;}
inline std::vector<unsigned> materialized(const Input& in){validate(in);std::vector<Planes> planes;for(std::size_t w=0;w<in.sequence.words.size();++w)planes.push_back(in.sequence.planes(w));std::vector<unsigned> out;for(auto p:planes)out.push_back(gc(p));return out;}
inline std::vector<unsigned> staged(const Input& in){validate(in);std::vector<unsigned> out;for(std::size_t begin=0;begin<in.sequence.words.size();begin+=16){std::vector<Planes> tile;for(std::size_t w=begin;w<std::min(begin+16,in.sequence.words.size());++w)tile.push_back(in.sequence.planes(w));for(auto p:tile)out.push_back(gc(p));}return out;}
}
