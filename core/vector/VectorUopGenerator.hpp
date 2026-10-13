// <VectorUopGenerator.hpp> -*- C++ -*-
//! \file VectorUopGenerator.hpp
#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/TreeNode.hpp"
#include "sparta/simulation/ParameterSet.hpp"
#include "sparta/pevents/PeventCollector.hpp"

#include "vector/VectorSequencer.hpp"

#include "InstArchInfo.hpp"
#include "Inst.hpp"
#include "FlushManager.hpp"
#include "decode/MavisUnit.hpp"

#include <optional>

namespace olympia
{

    /**
     * @file VectorUopGenerator.hpp
     * @brief TODO
     */
    class VectorUopGenerator : public sparta::Unit
    {
      public:
        //! \brief Parameters for VectorUopGenerator model
        class VectorUopGeneratorParameterSet : public sparta::ParameterSet
        {
          public:
            VectorUopGeneratorParameterSet(sparta::TreeNode* n) : sparta::ParameterSet(n) {}

            PARAMETER(bool, generate_tail_uops, false,
                      "Ignore VL and generate all uops even if they only contain tail elements")

            //! \brief Generate uops for widening vector instructions with two dests
            // PARAMETER(bool, widening_dual_dest, false,
            //     "Generate uops for widening vector instructions with two dests")
        };

        /**
         * @brief Constructor for VectorUopGenerator
         *
         * @param node The node that represents (has a pointer to) the VectorUopGenerator
         * @param p The VectorUopGenerator's parameter set
         */
        VectorUopGenerator(sparta::TreeNode* node, const VectorUopGeneratorParameterSet* p);

        //! \brief Name of this resource. Required by sparta::UnitFactory
        static constexpr char name[] = "vec_uop_gen";

        void setInst(const InstPtr & inst);

        InstPtr generateUop();

        uint64_t getNumUopsRemaining() const { return uops_.size() - num_uops_generated_; }

        void handleFlush(const FlushManager::FlushingCriteria &);

      private:
        void onBindTreeLate_() override;

        MavisType* mavis_facade_;

        VectorSequencer sequencer_;

        sparta::utils::ValidValue<InstPtr> current_inst_;

        std::vector<VectorSequencer::UopInfo> uops_;

        uint64_t num_uops_generated_ = 0;

        void reset_()
        {
            current_inst_.clearValid();
            uops_.clear();
            num_uops_generated_ = 0;
        }

        sparta::Counter vuops_generated_;

        sparta::pevents::PeventCollector<InstPEventPairs> vec_seq_inst_event_;
        sparta::pevents::PeventCollector<InstPEventPairs> vec_seq_uop_event_;

        InstPtr makeInst_(const InstPtr & inst, const VectorSequencer::UopInfo &);

        // Completion callback for vector uops (see Retire::completionCallback_)
        void completionCallback_(Inst* inst);

        // Dump debug content on failure
        void dumpDebugContent_(std::ostream & output) const override final;

        friend class VectorUopGeneratorTester;
    };

    class VectorUopGeneratorTester;
} // namespace olympia
