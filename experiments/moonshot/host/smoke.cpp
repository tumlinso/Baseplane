#include <bp_moon/source.hpp>
#include <iostream>
#include <random>

static void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
int main() {
    try {
        using namespace bp_moon;
        std::mt19937 random(2701);
        for (unsigned i = 0; i < 100; ++i) {
            u32 a = random(), b = random(), c = random();
            require(lut3(a,b,c,0x80) == (a&b&c), "LUT AND");
            require(lut3(a,b,c,0x96) == (a^b^c), "LUT XOR");
        }
        for (u32 mask : {u32{0},u32{1},u32{0x80000000},u32{0x5a51},~u32{0}})
            for (unsigned rank = 0; rank < popcount(mask); ++rank)
                require(rank_before(mask,select_bit(mask,rank)) == rank,"rank/select");
        CounterPlanes counters;
        for (unsigned i = 0; i < 15; ++i) counters.add(0x5a5a5a5a);
        require(!counters.overflow && counters.at(1)==15 && counters.at(0)==0,"counter planes");
        counters.add(0x5a5a5a5a);
        require(counters.overflow == 0x5a5a5a5a,"counter overflow");
        PackedFixture packed(std::string(31,'A')+"GNCGT");
        require(packed.words.size()==2 && !packed.is_valid(32) && !packed.is_valid(36),"packed validity");
        require(packed.planes(1).valid==14,"tail validity");
        require(PackedFixture("acgtN").valid[0]==15,"case validity");
        const auto left=summarize_sequence("ACGTAC"), right=summarize_sequence("TGGC");
        require(ce_moon::compose(left,right).to==summarize_sequence("ACGTACTGGC").to,"sequence concatenation");
        require(summarize_sequence("ACGN").to==base_transition('N').to,"invalid reset");
        require(summarize_sequence("acgt").to==summarize_sequence("ACGT").to,"case transition");
        auto support=canonical_support({{1,0,2},{1,5,6},{1,1,3},{2,1,3}});
        require(support.size()==3 && support[0].end==3 && support[1].begin==5,"source gaps");
        ExactInterner dictionary;
        require(dictionary.intern("ACGT")==dictionary.intern("ACGT") && dictionary.intern("ACGT")!=dictionary.intern("ACGN"),"exact identity");
        SourceMap source{17,3,100,2,Strand::forward,packed.original.size()};
        u64 output[2]{777,888};
        auto emitted=emit_valid_positions(packed,~u32{0},1,output,1,source);
        require(emitted.produced==3 && emitted.stored==1 && emitted.dropped==2,"bounded emission");
        require(output[0]==133 && output[1]==888,"capacity guard/source coordinate");
        emitted=emit_valid_positions(packed,~u32{0},1,nullptr,0,source);
        require(emitted.stored==0 && emitted.dropped==3,"count-only output");
        source.strand=Strand::reverse;
        require(source.coordinate(0)==135 && source.coordinate(35)==100,"reverse coordinate");
        bool overflow=false;
        source.origin=std::numeric_limits<u64>::max();
        try { (void)source.coordinate(0); } catch(const std::overflow_error&) { overflow=true; }
        require(overflow,"coordinate overflow");
        std::cout << "{\"status\":\"bp_foundation_smoke_pass\",\"case_groups\":10,\"gpu_executed\":false,\"biological_validation\":false}\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
