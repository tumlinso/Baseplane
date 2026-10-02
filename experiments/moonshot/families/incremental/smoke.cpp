#include "incremental.hpp"
#include <iostream>
using namespace bp_moon;
using namespace bp_moon::incremental;
void require(bool ok,const char* label){if(!ok)throw std::runtime_error(label);}
template<class F> void rejects(F f){bool yes=false;try{f();}catch(const std::exception&){yes=true;}require(yes,"expected rejection");}
int main(){try{
    PackedFixture a("ACGT"),b("ACGT"),invalid("NCGT");
    SourceMap first{1,2,100,1,Strand::forward,4},second{3,4,900,2,Strand::reverse,4};
    RepeatQuotient quotient;auto oa=quotient.add(a,first,7,0,0),ob=quotient.add(b,second,8,3,0),on=quotient.add(invalid,first,7,0,0);
    require(quotient.occurrences[oa].content==quotient.occurrences[ob].content,"E33 shared exact content");
    require(quotient.occurrences[oa].content!=quotient.occurrences[on].content,"E33 invalid token identity");
    require(a.words==invalid.words&&a.valid!=invalid.valid,"E33 identical packed payload different validity");
    require(quotient.occurrences[oa].state_version!=quotient.occurrences[ob].state_version,"E33 state versions separate");
    require(quotient.contextual_answer(oa)!=quotient.contextual_answer(ob),"E33 independent context answers");
    require(quotient.occurrences[ob].source.coordinate(0)==903,"E33 reverse provenance");
    auto overflow=first;overflow.origin=std::numeric_limits<u64>::max();rejects([&]{quotient.add(a,overflow,7,0,0);});
    std::cout<<"E33 contents="<<quotient.contents.size()<<" occurrences="<<quotient.occurrences.size()<<" exact_comparisons="<<quotient.exact_comparisons<<" contextual_answers="<<quotient.contextual_answer(oa)<<','<<quotient.contextual_answer(ob)<<'\n';

    // leaf0 -> effect2 -> parent4 -> query5; independent leaf1 -> effect3.
    // leaf0 also invalidates directory6 -> posting-query7 -> query5.
    DirtyCone cone({{2},{3},{4},{},{5},{},{7},{5}},{"ACGT","TTTT"},{first,second});
    auto edited_source=first;++edited_source.version;
    auto dirty=cone.replace(0,6,"GCGT",edited_source);
    rejects([&]{cone.replace(0,6,"ACGT",first);});
    require(dirty==std::vector<unsigned>({0,2,4,5,6,7}),"E34 dirty ancestors and structural cone");
    require(!cone.directory.count("ACGT")&&cone.directory.at("GCGT").count(0),"E34 old posting removed new posting added");
    require(cone.directory.at("TTTT")==std::set<unsigned>{1},"E34 sibling membership preserved");
    auto cycle_parents=std::vector<std::vector<unsigned>>{{1},{0}};
    DirtyCone cyclic(cycle_parents,{"ACGT"},{first});require(cyclic.replace(0,1,"GCGT",edited_source)==std::vector<unsigned>({0,1}),"E34 cyclic fixed point terminates");
    auto old_effect=summarize_sequence("ACGT"),updated=summarize_sequence(cone.keys[0]);
    // DFA forgets all but its last two bases: changing an earlier base can leave
    // the value equal while structural memberships still require invalidation.
    require(old_effect.to==updated.to,"E34 equal value does not cancel membership invalidation");
    std::cout<<"E34 dirty_nodes="<<dirty.size()<<" preserved_sibling=1 directory_keys="<<cone.directory.size()<<'\n';

    PackedFixture sequence("GCAAAAAT");SourceMap source{4,5,1000,10,Strand::forward,8};QueryIdentity query{2,3,4,5};
    auto memo=Memo::count(sequence,source,query,{0,4});auto current=source;++current.version;
    require(memo.reusable(current,query,{{10,11,7,1,1}}),"E35 outside support retained");
    require(!memo.reusable(current,query,{{10,11,2,1,1}}),"E35 negative evidence invalidated");
    auto weights=query;++weights.weights;require(!memo.reusable(source,weights,{}),"E35 weights invalidated");
    auto changed_query=query;++changed_query.query;require(!memo.reusable(source,changed_query,{}),"E35 query invalidated");
    auto state=query;++state.cell_state;require(!memo.reusable(source,state,{}),"E35 state invalidated");
    auto rep=query;++rep.representation;require(!memo.reusable(source,rep,{}),"E35 representation invalidated");
    require(!memo.reusable(current,query,{}),"E35 missing edit chain invalidated");
    require(!memo.reusable(current,query,{{10,12,7,1,1}}),"E35 skipped edit version invalidated");
    auto shifted=current;shifted.length=9;require(!memo.reusable(shifted,query,{{10,11,0,0,1}}),"E35 shifted support invalidated");
    require(memo.reusable(shifted,query,{{10,11,8,0,1}}),"E35 appended outside forward support retained");
    auto wrong=source;++wrong.origin;require(!memo.reusable(wrong,query,{}),"E35 origin changed invalidated");
    auto reverse=source;reverse.strand=Strand::reverse;auto rm=Memo::count(sequence,reverse,query,{0,4});auto rc=reverse;++rc.version;++rc.length;
    require(!rm.reusable(rc,query,{{10,11,8,0,1}}),"E35 reverse coordinate shift invalidated");
    CountPlan plan{{0,4}};PackedFixture changed("GCGAAAAT");require(plan.execute(changed)!=memo.answer,"E35 plan reruns rather than returning cached result");
    std::cout<<"E35 cached_answer="<<memo.answer<<" recomputed_after_negative_edit="<<plan.execute(changed)<<" inspected_support_bases=4\n";

    PackedFixture repeated("ACGTACGT");SourceMap map{9,10,200,1,Strand::forward,8};Grammar grammar;
    auto phrase_a=grammar.add(repeated,map,{0,4}),phrase_b=grammar.add(repeated,map,{4,8});
    require(phrase_a==phrase_b,"E36 repeated phrase shared");auto before=grammar.compositions;
    auto root=grammar.add(repeated,map,{0,8});require(grammar.compositions==before+1,"E36 multiscale pair reuse");
    require(grammar.nodes[root].effect.to==summarize_sequence(repeated.original).to,"E36 ordered effect scalar comparison");
    require(grammar.expand(root)==repeated.original,"E36 exact expansion");
    require(grammar.coordinates(0)==std::vector<u64>({200,201,202,203})&&grammar.coordinates(1)==std::vector<u64>({204,205,206,207}),"E36 distinct occurrence spans");
    Grammar static_pass;auto static_id=static_pass.add(repeated,map,{0,8});require(static_pass.nodes[static_id].effect.to==grammar.nodes[root].effect.to,"E36 static versus incremental pair effect");
    auto invalid_id=grammar.add(invalid,first,{0,4});require(grammar.expand(invalid_id)=="NCGT","E36 invalid identity exact expansion");
    auto opposite=grammar.pair(grammar.token('G'),grammar.token('A'));
    auto ordered=grammar.pair(grammar.token('A'),grammar.token('G'));require(opposite!=ordered,"E36 ordered tuple identity");
    rejects([&]{grammar.add(repeated,map,{0,0});});
    std::cout<<"E36 grammar_nodes="<<grammar.nodes.size()<<" incremental_compositions="<<grammar.compositions<<" static_compositions="<<static_pass.compositions<<" shared_phrase_occurrences=2\n";
    std::cout<<"incremental semantic checks passed; CPU fixture only\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
