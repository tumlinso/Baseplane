#include "hypotheses.hpp"
#include <iostream>
#include <type_traits>
using namespace bp_moon::hypotheses;
static void require(bool v,const char* m){if(!v)throw std::runtime_error(m);}
template<class E,class F>static void rejects(F f){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,"invalid input accepted");}
static SourceRef source(std::string text,bp_moon::u64 origin=100,Strand strand=Strand::forward){return std::make_shared<const Source>(7,origin,3,strand,std::move(text));}
int main(){try{
    static_assert(std::is_const_v<SourceRef::element_type>,"source is immutable");
    static_assert(std::is_const_v<Parse::element_type>,"parse interpretations are immutable");
    // E37: same exact bases and common prefix, alternative segment boundaries.
    auto shared=source("ACGTACGT",100,Strand::reverse);
    auto prefix=extend(shared,{},2);
    auto fine_middle=extend(shared,prefix,4),coarse_middle=extend(shared,prefix,6);
    auto fine=extend(shared,fine_middle,8),coarse=extend(shared,coarse_middle,8);
    require(fine->parent->parent==coarse->parent->parent,"prefix node sharing");
    std::array<double,4> rows{.1,.35,.65,.9},targets{0,0,1,1},reverse_targets{1,1,0,0};
    std::array<double,2> positive{},negative{};
    auto fit=ce_moon::learning::fit_logistic(rows.data(),targets.data(),4,1,positive.data());
    auto other_fit=ce_moon::learning::fit_logistic(rows.data(),reverse_targets.data(),4,1,negative.data());
    require(fit.final_loss<fit.initial_loss&&other_fit.final_loss<other_fit.initial_loss,"CE fitted scorer improves synthetic loss");
    auto wider=retain_beam({fine,coarse},positive,2),narrower=retain_beam({fine,coarse},negative,2);
    require(wider.retained[0]==coarse&&narrower.retained[0]==fine,"query-dependent ambiguity resolution");
    require(!wider.approximate&&wider.dropped==0,"unpruned alternatives");
    auto pruned=retain_beam({fine,coarse},positive,1);
    require(pruned.approximate&&pruned.dropped==1&&pruned.retained[0]!=narrower.retained[0],"beam may lose later answer");
    auto spans=parse_supports(fine);
    require(spans.size()==3&&spans[0].begin==100&&spans[0].end==102&&spans[1].begin==102&&spans[2].end==108&&spans[0].strand==Strand::reverse&&spans[0].epoch==3,"exact parse support");
    require(shared->packed.original=="ACGTACGT"&&fine->parent==fine_middle,"selection preserves source and branches");
    rejects<std::invalid_argument>([&]{extend(source("AAAA"),prefix,3);});
    rejects<std::invalid_argument>([&]{retain_beam({fine},positive,0);});
    auto other_source=source("ACGTACGT");
    auto other_branch=extend(other_source,extend(other_source,{},2),8);
    rejects<std::invalid_argument>([&]{retain_beam({fine,other_branch},positive,2);});
    rejects<std::invalid_argument>([]{validate_local_size(std::size_t(0x80000000u));});
    // E38: role binding preserves this order example, but not exact identity.
    require(role_sketch(source("AC"))!=role_sketch(source("CA")),"role-bound order");
    std::map<unsigned,SourceRef> directory;SourceRef collision_a,collision_b;
    const std::string bases="ACGT";
    for(unsigned k=0;k<1024&&!collision_a;++k){
        auto index=k;std::string text(5,'A');for(char& c:text){c=bases[index%4];index/=4;}
        auto item=source(text,k*10);unsigned key=role_sketch(item);auto found=directory.find(key);
        if(found!=directory.end()&&found->second->packed.original!=text){collision_a=found->second;collision_b=item;}
        else directory.emplace(key,item);
    }
    require(collision_a&&collision_b,"constructed sketch collision");
    auto rejected=verify_sketch(collision_a,collision_b);
    require(rejected.nominated&&!rejected.exact_equal&&rejected.replayed_bases==10,"collision triggers exact replay");
    auto accepted=verify_sketch(collision_a,collision_a);
    require(accepted.nominated&&accepted.exact_equal,"identical source verifies");
    // E39: abundance and order have distinct sufficient summaries.
    Portfolio ac(source("AC")),ca(source("CA")),invalid(source("ANaC"));
    auto abundance_a=ask(ac,Query::adenine_count),abundance_b=ask(ca,Query::adenine_count);
    require(abundance_a.value==abundance_b.value&&abundance_a.route==Route::count,"equal histogram query");
    require(ask(ac,Query::terminal_state).value!=ask(ca,Query::terminal_state).value,"different effect query");
    auto replay=ask(ac,Query::exact_motif,"AC");
    require(replay.value==1&&replay.route==Route::source_replay&&replay.replayed_bases==2,"unsupported summary question replays source");
    require(invalid.invalid==1&&ask(invalid,Query::adenine_count).value==2,"invalid payload excluded from counts");
    require(ask(invalid,Query::exact_motif,"AA").value==0&&ask(invalid,Query::exact_motif,"aC").value==1,"validity and canonical replay");
    rejects<std::invalid_argument>([&]{ask(invalid,Query::exact_motif,"AN");});
    // E40: only demonstrated failure sites gain a cached effect residual.
    RepairIndex initial({ac,ca,Portfolio(source("GT")),invalid});
    require(initial.terminal(0).route==Route::source_replay,"unrepaired query replay");
    RepairIndex::RepairReceipt repair{};auto refined=initial.repaired(0,1,&repair);
    require(repair.sites==2&&repair.failed_question_separated&&refined.residual.size()==2,"counterexample sites repaired");
    require(initial.residual.empty(),"previous interpretation index immutable");
    require(refined.terminal(0).value!=refined.terminal(1).value&&refined.terminal(0).route==Route::effect,"effect residual separates failure");
    require(refined.terminal(2).route==Route::source_replay&&refined.terminal(2).replayed_bases==2,"unseen site remains replayable");
    auto unchanged=initial.repaired(0,2,&repair);require(repair.sites==0&&unchanged.residual.empty(),"different histogram is not failed coarse question");
    std::array<bool,4> explored{};for(unsigned seed=0;seed<100;++seed)explored[initial.exploration_site(seed)]=true;
    require(std::all_of(explored.begin(),explored.end(),[](bool b){return b;}),"exploration includes unrepaired sites");
    rejects<std::out_of_range>([&]{initial.repaired(0,100);});
    rejects<std::overflow_error>([]{source("AC",std::numeric_limits<bp_moon::u64>::max());});
    std::cout<<"{\"status\":\"hypotheses_pass\",\"cards\":[\"E37\",\"E38\",\"E39\",\"E40\"],\"fit_initial_loss\":"<<fit.initial_loss<<",\"fit_final_loss\":"<<fit.final_loss<<",\"collision_left\":\""<<collision_a->packed.original<<"\",\"collision_right\":\""<<collision_b->packed.original<<"\",\"repaired_sites\":2,\"gpu_run\":false}\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
