#include <bp_moon/source.hpp>
#include <iostream>

int main() {
    try {
        const std::string dna="ACGTACGTTTTTACGNACGTACGTGGGGACGT";
        bp_moon::PackedFixture source(dna);
        bp_moon::SourceMap mapping{1,1,100,1,bp_moon::Strand::forward,dna.size()};
        std::vector<double> effects;
        for(std::size_t begin=0; begin<dna.size(); begin+=4)
            effects.push_back(bp_moon::summarize_sequence(dna.substr(begin,4)).to[0]);
        ce_moon::ResidualTree hierarchy(effects);
        std::size_t visited=0;
        auto requests=hierarchy.above(8.,&visited);
        bp_moon::ExactInterner dictionary;
        for(std::size_t begin=0; begin<dna.size(); begin+=4)
            dictionary.intern(dna.substr(begin,4));
        std::cout << "Exact input: " << dna << "\nToy windows: " << effects.size()
                  << "; distinct exact strings: " << dictionary.dictionary.size()
                  << "\nQuery visited " << visited << " nodes; exact source revisits: " << requests.size() << '\n';
        for(auto window:requests) {
            const auto begin=window*4, end=std::min(dna.size(),begin+4);
            std::cout << "source=" << mapping.source_id << " contig=" << mapping.contig
                      << " coordinate=" << mapping.coordinate(begin) << " version=" << mapping.version
                      << " exact=" << dna.substr(begin,end-begin)
                      << " effect=" << hierarchy.reconstruct(window) << '\n';
        }
        std::cout << "Sequence-derived toy effects with explicit source recovery; no learned biology or timing claim.\n";
    } catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
