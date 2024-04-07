#include "emptiness_check.hpp"
#include <tuple>

// kofola
#include "complement_tela.hpp"
#include "util.hpp"
#include "decomposer.hpp"
#include "complement_sync.hpp"
#include "inclusion_check.hpp"
#include "hyperltl_mc.hpp"

// Spot
#include <spot/twaalgos/postproc.hh>
#include <spot/twaalgos/product.hh>
#include <spot/twaalgos/complete.hh>

// standard library
#include <queue>
#include <utility>

namespace kofola {
    emptiness_check::emptiness_check(abstract_successor *as, int type, bool use_early_subsums):
    abstr_succ_(as), type_(type), use_early_subsums_(use_early_subsums)
    {
    }

    bool emptiness_check::empty() {
        auto init_states = abstr_succ_->get_initial_states();
        for (const auto& state: init_states) {
            dfs_num_.insert({state, UNDEFINED});
            on_stack_.insert({state, false});
        }

        for(const auto& init: init_states) {
            if (dfs_num_.at(init) == UNDEFINED) {
                tarjan_is_empty(init, spot::acc_cond::mark_t());
                if(decided_) {
                    std::cout << "---" << cnt_ << "---";
                    return empty_;
                }
            }
        }

        std::cout << "---" << cnt_ << "---";
        return empty_;
    }

    void emptiness_check::tarjan_is_empty(const std::shared_ptr<abstract_successor::mstate> &src_mstate, spot::acc_cond::mark_t path_cond) {
        /// STRONGCONNECT
        // abstr_succ_->print_mstate(src_mstate);
        cnt_++;
        if(use_early_subsums_) {
            if (abstr_succ_->is_accepting(path_cond)) {
                auto tmp = spot::acc_cond::mark_t();
                for (auto it = tarjan_stack_.rbegin(); it != tarjan_stack_.rend(); ++it) {
                    const auto &s = *it;
                    if (abstr_succ_->is_accepting(tmp) && abstr_succ_->subsum_less(s, src_mstate)) {
                        abstr_succ_->subsum_less(s, src_mstate);
                        decided_ = true;
                        empty_ = false;
                        return;
                    }
                    tmp |= s->get_acc();
                }
            }
        }

        SCCs_.push(src_mstate);
        dfs_num_[src_mstate] = index_;
        index_++;
        tarjan_stack_.push_back(src_mstate);
        on_stack_[src_mstate] = true;

        auto succs = abstr_succ_->get_succs(src_mstate);

        for (auto &dst_mstate: succs) {
            // checking emptiness of curr dst_mstate
            if(use_early_subsums_) {
                bool is_empty = false;
                for (const auto &empty_state: empty_lang_states_) {
                    if (abstr_succ_->subsum_less(dst_mstate, empty_state)) {
                        is_empty = true;
                        break;
                    }
                }
                if (is_empty)
                    continue;
            }
            // end of checking emptiness

            // init structures
            if(dfs_num_.count(dst_mstate) == 0)
            {
                dfs_num_.insert({dst_mstate, UNDEFINED});
                on_stack_.insert({dst_mstate, false});
            }

            if (dfs_num_[dst_mstate] == UNDEFINED)
            {
                tarjan_is_empty( dst_mstate, (path_cond | dst_mstate->get_acc()) );

                if(decided_)
                    return;
            } else if(on_stack_[dst_mstate]) {
                spot::acc_cond::mark_t cond = dst_mstate->get_acc();

                // merge acc. marks
                std::shared_ptr<abstract_successor::mstate> tmp;
                do {
                    tmp = SCCs_.top(); SCCs_.pop();
                    bool root_encountered = dst_mstate->get_encountered();
                    if(root_encountered || dfs_num_[tmp] > dfs_num_[dst_mstate])
                        cond = (cond |= tmp->get_acc());
                    if(abstr_succ_->is_accepting(cond)){
                        decided_ = true;
                        empty_ = false;
                        //abstr_succ_->print_mstate(dst_mstate);
                        return;
                    }
                } while(dfs_num_[tmp] > dfs_num_[dst_mstate]);
                dst_mstate->set_encountered(true); // mark visited root
                tmp->set_acc(cond);
                SCCs_.push(tmp);
            }
        }

        if (SCCs_.top() == (src_mstate)) {
            SCCs_.pop();
            std::shared_ptr<abstract_successor::mstate> tmp;

            do {
                tmp = tarjan_stack_.back(); tarjan_stack_.pop_back();
                on_stack_[tmp] = false;
                empty_lang_states_.emplace_back(tmp); // when here, each state has empty language, otherwise we would have ended
            } while (src_mstate != tmp);
        }
    }
}// namespace KOFOLA