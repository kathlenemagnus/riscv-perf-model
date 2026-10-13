#include "vector/VectorUopGenerator.hpp"
#include "Inst.hpp"
#include "CoreUtils.hpp"
#include "mavis/Mavis.h"
#include "mavis/InstMetaData.h"
#include "sparta/utils/LogUtils.hpp"

namespace olympia
{
    constexpr char VectorUopGenerator::name[];

    VectorUopGenerator::VectorUopGenerator(sparta::TreeNode* node,
                                           const VectorUopGeneratorParameterSet* p) :
        sparta::Unit(node),

        sequencer_(coreutils::getCoreExtensions(node)->getParameterAs<uint32_t>("vlen"),
                   coreutils::getVlenDlenRatio(node),
                   p->generate_tail_uops),
        vuops_generated_(&unit_stat_set_, "vector_uops_generated",
                         "Number of vector uops generated", sparta::Counter::COUNT_NORMAL),
        vec_seq_inst_event_("VEC_SEQ", getContainer(), getClock()),
        vec_seq_uop_event_("VEC_SEQ_UOP", getContainer(), getClock())
    {
    }

    void VectorUopGenerator::onBindTreeLate_()
    {
        mavis_facade_ = getMavis(getContainer());
    }

    void VectorUopGenerator::setInst(const InstPtr & inst)
    {
        sparta_assert(current_inst_.isValid() == false,
                      "Cannot start generating uops for a new vector instruction, "
                      "current instruction has not finished: "
                          << current_inst_);

        current_inst_ = inst;

        // Sequence inst
        uops_ = sequencer_.sequenceInst(current_inst_.getValue());

        // TODO: For loads and stores, make sure that the instruction has an address for
        // every element.
        //if (inst->isLoadStoreInst())

        ILOG(current_inst_ << " (" << inst->getVectorGlobalConfig() << ") is being split into "
                           << uops_.size() << " UOPs");

        current_inst_.getValue()->setNumVectorUOps(uops_.size());

        vec_seq_inst_event_.collect(*inst);

        // Nothing to generate
        if (uops_.size() == 0)
        {
            reset_();
        }
    }

    InstPtr VectorUopGenerator::generateUop()
    {
        sparta_assert(current_inst_.isValid(),
                      "Cannot generate uops, current instruction is not set");
        const auto inst = current_inst_.getValue();
        const auto uop_gen_type = inst->getUopGenType();
        sparta_assert(uop_gen_type <= InstArchInfo::UopGenType::NONE,
                      "Inst: " << current_inst_ << " uop gen type is unknown");

        // Generate uop
        const VectorSequencer::UopInfo & uop_info = uops_.at(num_uops_generated_);
        const InstPtr uop = makeInst_(inst, uop_info);

        // setting UOp instructions to have the same UID and PID as parent instruction
        uop->setUniqueID(inst->getUniqueID());
        uop->setProgramID(inst->getProgramID());
        uop->setUOpID(num_uops_generated_ + 1);

        // Set child uop to point to parent's global vector config
        const vector_types::VectorGlobalConfig* vec_global_config = inst->getVectorGlobalConfig();
        uop->setVectorGlobalConfig(*vec_global_config);

        // Get load/store data size and nfields from parent
        if (inst->isLoadStoreInst())
        {
            uop->setNfields(inst->getNfields());
        }

        // Set weak pointer to parent vector instruction
        sparta::SpartaWeakPointer<olympia::Inst> parent_weak_ptr = inst;
        uop->setUOpParent(parent_weak_ptr);

        // Initialize the vector instruction state in the child uop
        uop->initVectorInstState(uop_info.getStartElemNum(), uop_info.getActiveElemMask(),
                                 uop_info.getDlenIdx());

        // TODO: Need to get the virtual addresses of all active elements in this uop from STF/JSON
        // After considering the tail and mask, how many active elements are in this uop?
        const vector_types::VectorInstStatePtr & vec_inst_state = uop->getVectorInstState();
        //const uint32_t num_elems_this_uop = vec_inst_state->getNumElemsThisUop();
        //if (uop->isLoadStoreInst() && (num_elems_this_uop > 0))

        ++num_uops_generated_;
        ++vuops_generated_;

        // Handle last uop
        if (num_uops_generated_ == uops_.size())
        {
            reset_();
        }

        ILOG("Generated uop: " << uop << "(" << vec_inst_state << ")");

        vec_seq_uop_event_.collect(*uop);

        return uop;
    }

    InstPtr VectorUopGenerator::makeInst_(const InstPtr & inst,
                                          const VectorSequencer::UopInfo & uop_info)
    {
        // Pass on any special fields from parent to child
        const auto & specials = inst->getSpecialFields();

        // Create uop
        InstPtr uop;
        if (current_inst_.getValue()->hasImmediate())
        {
            mavis::ExtractorDirectOpInfoList ex_info(inst->getMnemonic(), uop_info.getSrcs(),
                                                     uop_info.getDests(), specials,
                                                     inst->getImmediate());
            uop = mavis_facade_->makeInstDirectly(ex_info, getClock());
        }
        else
        {
            mavis::ExtractorDirectOpInfoList ex_info(inst->getMnemonic(), uop_info.getSrcs(),
                                                     uop_info.getDests(), specials);
            uop = mavis_facade_->makeInstDirectly(ex_info, getClock());
        }

        return uop;
    }

    void VectorUopGenerator::handleFlush(const FlushManager::FlushingCriteria & flush_criteria)
    {
        if (current_inst_.isValid() && flush_criteria.includedInFlush(current_inst_))
        {
            current_inst_.clearValid();
            uops_.clear();
            num_uops_generated_ = 0;
        }
    }

    void VectorUopGenerator::dumpDebugContent_(std::ostream & output) const
    {
        output << "Current Vector Instruction: " << current_inst_ << std::endl;
        output << "Num Uops Generated: " << num_uops_generated_ << std::endl;
        output << "Num Uops Remaining: " << (uops_.size() - num_uops_generated_) << std::endl;
    }
} // namespace olympia
