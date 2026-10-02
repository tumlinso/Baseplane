#include "compiler.hpp"
#include <iostream>

using namespace bp_moon;
using namespace bp_moon::compiler;
void check(bool ok,const char* why) { if(!ok)throw std::runtime_error(why); }
template<class F>void rejects(F f) {
  bool thrown=false;try{f();}catch(const std::invalid_argument&){thrown=true;}
  check(thrown,"invalid contract accepted");
}
int main() {
  auto target=training_labels();
  auto teacher=ce::fit_truth_table(target,200,0.5,17);
  auto hard=ce::harden(teacher,23);
  unsigned expected=0;
  for(unsigned row=0;row<8;++row) {
    expected|=unsigned(((row&4)&&(row&2)) || (row&1))<<row;
    check(hard.predict(row)==bool(target[row]),"exhaustive sequence-derived training table");
  }
  check(hard.immediate==expected,"sequence question hardening");
  // Held-out DNA contexts use all 64 canonical triplets, including features not
  // represented by the eight fitting sequences; N barriers have invalid payloads.
  std::string heldout;
  const std::string bases="ACGT";
  for(char a:bases)for(char b:bases)for(char c:bases)heldout+=std::string{a,b,c}+'N';
  heldout+="acgTaNCGTACN";
  PackedFixture sequence(heldout);
  // Corrupt invalid packed payload intentionally; validity still controls outputs.
  for(std::size_t i=0;i<heldout.size();++i)if(!sequence.is_valid(i))
    sequence.words[i/32]|=u64{3}<<(2*(i%32));
  SourceMap source{101,9,1000,7,Strand::forward,heldout.size()};
  Provenance provenance{17,23,3,4,0};
  std::vector<Hit> hits(heldout.size());
  auto counts=hardened_query(sequence,source,hard,provenance,hits.data(),hits.size());
  u64 scalar_count=0;
  for(std::size_t i=0;i<heldout.size();++i) {
    auto x=features(sequence,i);
    if(scalar_question(x)) {
      check(hits[scalar_count].anchor==source.coordinate(i),"held-out exact anchor");
      check(hits[scalar_count].support_count==3,"triplet support");
      for(unsigned j=0;j<3;++j)check(hits[scalar_count].support[j]==source.coordinate(i-1+j),"exact source supports");
      check(hits[scalar_count].source_id==101 && hits[scalar_count].source_version==7 &&
            hits[scalar_count].contig==9 && hits[scalar_count].provenance.weights==17 &&
            hits[scalar_count].provenance.circuit==23,"source/model provenance");
      ++scalar_count;
    }
    if(x.valid)check(hard.predict(x.row)==(teacher.predict(x.row)>=0.5),"held-out teacher/hard decision");
  }
  check(counts.produced==scalar_count && counts.stored==scalar_count && !counts.dropped,"held-out packed equivalence");
  Hit one;
  auto limited=hardened_query(sequence,source,hard,provenance,&one,1);
  check(limited.produced==scalar_count && limited.stored==1 && limited.dropped==scalar_count-1,"bounded output");
  auto dry=hardened_query(sequence,source,hard,provenance,nullptr,0);
  check(dry.produced==scalar_count && dry.stored==0 && dry.dropped==scalar_count,"zero capacity accounting");
  auto wrong=provenance; ++wrong.circuit;
  rejects([&]{hardened_query(sequence,source,hard,wrong,hits.data(),hits.size());});
  SourceMap wrongsource=source; --wrongsource.length;
  rejects([&]{hardened_query(sequence,wrongsource,hard,provenance,hits.data(),hits.size());});
  rejects([&]{hardened_query(sequence,source,hard,provenance,nullptr,1);});

  // Real 31/32 word seam and reverse coordinate support, no halo duplication.
  std::string seam(70,'A');seam[31]='C';seam[32]='T';
  PackedFixture seam_sequence(seam);
  SourceMap reverse{202,10,5000,8,Strand::reverse,seam.size()};
  std::vector<Hit> seam_hits(seam.size());
  auto seam_counts=hardened_query(seam_sequence,reverse,hard,provenance,seam_hits.data(),seam_hits.size());
  unsigned seam_anchor_count=0;
  for(u64 h=0;h<seam_counts.stored;++h)if(seam_hits[h].anchor==reverse.coordinate(31)) {
    ++seam_anchor_count;
    check(seam_hits[h].strand==Strand::reverse && seam_hits[h].support[0]==reverse.coordinate(30) &&
          seam_hits[h].support[2]==reverse.coordinate(32),"reverse seam support");
  }
  check(seam_anchor_count==1,"word seam anchor duplicate/missing");
  check(features(seam_sequence,31).valid && features(seam_sequence,31).row==7,"seam features");

  // E30 same sequence, two learned synthetic states, different exact predicates.
  auto model=train_banks(29);
  std::vector<Hit> ac_hits(heldout.size()),gt_hits(heldout.size());
  auto ac=bank_query(sequence,source,model,-0.8,31,ac_hits.data(),ac_hits.size());
  auto gt=bank_query(sequence,source,model,0.8,32,gt_hits.data(),gt_hits.size());
  check(ac.selection.index==0 && gt.selection.index==1,"state-conditioned bank");
  check(ac.counts.produced && gt.counts.produced,"bank source outputs");
  for(auto pair:{std::make_pair(&ac,&ac_hits),std::make_pair(&gt,&gt_hits)}) {
    unsigned first=pair.first->selection.index?2:0, second=first+1;
    for(u64 k=0;k<pair.first->counts.stored;++k) {
      const auto& h=pair.second->at(k);std::size_t i=std::size_t(h.anchor-source.origin);
      check(base_code(sequence,i)==first && base_code(sequence,i+1)==second,"exact bank motif");
      check(sequence.is_valid(i)&&sequence.is_valid(i+1)&&h.support_count==2,"bank validity/support");
      check(h.source_id==source.source_id && h.provenance.bank==pair.first->selection.index &&
            h.provenance.state==pair.first->selection.state_version && h.provenance.weights==29,"bank selection provenance");
    }
  }
  check(ac_hits.front().anchor!=gt_hits.front().anchor,"banks silently ask identical question");

  // E31 source-domain typed sequence question evaluated independently on held-out
  // exact DNA, rather than trusting provider truth-table equality alone.
  auto compiled=compile_question(501);
  check(compiled.rewrite.before==3 && compiled.rewrite.after==2,"Boolean extraction cost");
  u64 rewrite_matches=0;
  for(std::size_t w=0;w<sequence.words.size();++w) {
    const auto f=planes(sequence,w);u32 mask=rewritten_mask(compiled,f,501);
    for(unsigned lane=0;lane<32;++lane) {
      auto x=features(sequence,w*32+lane);
      bool expected_question=x.valid && x.gc && (x.previous_a||x.next_t);
      check(bool(mask&(u32{1}<<lane))==expected_question,"held-out rewritten sequence decisions");
      if(expected_question)++rewrite_matches;
    }
  }
  auto rewritten=rewritten_query(compiled,sequence,source,501,41,hits.data(),hits.size());
  check(rewritten.produced==rewrite_matches && rewritten.stored==rewrite_matches,"rewritten source emission");
  for(u64 k=0;k<rewritten.stored;++k) {
    const auto& h=hits[k];auto i=std::size_t(h.anchor-source.origin);
    auto x=features(sequence,i);
    check(x.valid && x.gc && (x.previous_a||x.next_t) && h.source_version==source.version &&
          h.support[0]==source.coordinate(i-1) && h.support[2]==source.coordinate(i+1) &&
          h.provenance.query==41,"rewritten source/parameter provenance");
  }
  rejects([&]{rewritten_mask(compiled,planes(sequence,0),502);});
  check(!ce::reassociation_allowed(ce::NumericalMode::strict_float),"strict float rewrite accepted");

  // E32 parameter/query/state/source versions and identity independently invalidate.
  auto cached=prepare(source,teacher,3,4,23);
  auto fresh=guarded_query(cached,sequence,source,teacher,3,4,hits.data(),hits.size());
  check(fresh.specialization_reused && fresh.counts.produced==scalar_count && !fresh.source_revisits,"fresh prepared query");
  auto query_changed=guarded_query(cached,sequence,source,teacher,4,4,hits.data(),hits.size());
  check(!query_changed.specialization_reused && query_changed.source_revisits && query_changed.counts.produced==scalar_count,"query fallback");
  auto state_changed=guarded_query(cached,sequence,source,teacher,3,5,hits.data(),hits.size());
  check(!state_changed.specialization_reused && state_changed.counts.produced==scalar_count,"state fallback");
  auto changed_source=source; ++changed_source.version;
  auto version_changed=guarded_query(cached,sequence,changed_source,teacher,3,4,hits.data(),hits.size());
  check(!version_changed.specialization_reused && hits[0].source_version==changed_source.version,"source version fallback");
  changed_source=source; ++changed_source.source_id;
  auto identity_changed=guarded_query(cached,sequence,changed_source,teacher,3,4,hits.data(),hits.size());
  check(!identity_changed.specialization_reused && hits[0].source_id==changed_source.source_id,"source identity fallback");
  changed_source=source; ++changed_source.origin;
  auto moved=guarded_query(cached,sequence,changed_source,teacher,3,4,hits.data(),hits.size());
  check(!moved.specialization_reused && hits[0].anchor==one.anchor+1,"coordinate origin fallback");
  auto changed_teacher=teacher; ++changed_teacher.weight_version;
  for(auto& z:changed_teacher.logits)z=-10;
  auto weights_changed=guarded_query(cached,sequence,source,changed_teacher,3,4,hits.data(),hits.size());
  check(!weights_changed.specialization_reused && weights_changed.counts.produced==0 && weights_changed.source_revisits,"new weights silently used old circuit");
  // Empty and invalid inputs retain no biological answers.
  PackedFixture invalid("NNNN");SourceMap invalid_source{3,1,0,1,Strand::forward,4};
  check(!hardened_query(invalid,invalid_source,hard,provenance,nullptr,0).produced,"invalid payload biology");
  PackedFixture empty("");SourceMap empty_source{3,1,0,1,Strand::forward,0};
  check(!hardened_query(empty,empty_source,hard,provenance,nullptr,0).produced,"empty input");
  std::cout << "E29 LUT " << expected << ", held-out hits " << scalar_count << ", seam anchors " << seam_anchor_count
            << "; E30 AC/GT hits " << ac.counts.produced << '/' << gt.counts.produced
            << "; E31 cost " << compiled.rewrite.before << " -> " << compiled.rewrite.after
            << ", matches " << rewrite_matches << "; E32 fresh reuse + six invalidations pass\n";
}
