// <VectorSequencer.hpp> -*- C++ -*-
//! \file VectorSequencer.hpp
#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/TreeNode.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "InstArchInfo.hpp"
#include "Inst.hpp"
#include "FlushManager.hpp"
#include "decode/MavisUnit.hpp"

#include <optional>
#include <boost/dynamic_bitset.hpp>

namespace olympia
{

    /**
     * @file VectorSequencer.hpp
     * @brief TODO
     */
    class VectorSequencer
    {
      public:
        class UopInfo
        {
          public:
            UopInfo(const mavis::OperandInfo::ElementList & srcs,
                    const mavis::OperandInfo::ElementList & dests, const uint32_t start_elem_num,
                    const boost::dynamic_bitset<> & active_elem_mask, const uint32_t dlen_idx) :
                srcs_(srcs),
                dests_(dests),
                starting_elem_num_(start_elem_num),
                active_elem_mask_(active_elem_mask),
                dlen_idx_(dlen_idx)
            {
            }

            const mavis::OperandInfo::ElementList & getSrcs() const { return srcs_; }

            const mavis::OperandInfo::ElementList & getDests() const { return dests_; }

            uint32_t getStartElemNum() const { return starting_elem_num_; }

            const boost::dynamic_bitset<> & getActiveElemMask() const { return active_elem_mask_; }

            uint32_t getDlenIdx() const { return dlen_idx_; }

          private:
            const mavis::OperandInfo::ElementList srcs_;
            const mavis::OperandInfo::ElementList dests_;
            const uint32_t starting_elem_num_;
            const boost::dynamic_bitset<> active_elem_mask_; // 1=active, 0=inactive/masked/tail
            const uint32_t dlen_idx_;
        };

        class Modifier
        {
          public:
            Modifier(std::string name, uint32_t value) : name_{name}, data_{value} {}

            std::string getName() const { return name_; }

            uint32_t getValue() const { return data_; }

            void setValue(uint32_t newValue) { data_ = newValue; }

          private:
            std::string name_;
            uint32_t data_;
        };

        /**
         * @brief Constructor for VectorSequencer
         */
        VectorSequencer(const uint32_t vlen, const uint32_t vlen_dlen_ratio, const bool sequence_tail_uops);

        std::vector<UopInfo> sequenceInst(const InstPtr &);

      private:
        const uint32_t vlen_;
        const uint32_t vlen_dlen_ratio_;

        const bool sequence_tail_uops_;

        typedef std::pair<mavis::OperandInfo::ElementList, mavis::OperandInfo::ElementList>
            MavisOperands;
        typedef std::function<MavisOperands(VectorSequencer*)> UopGenFunctionType;
        typedef std::map<InstArchInfo::UopGenType, UopGenFunctionType> UopGenFunctionMapType;
        UopGenFunctionMapType uop_gen_function_map_;

        sparta::utils::ValidValue<InstPtr> current_inst_;
        std::vector<Modifier> current_inst_modifiers_;

        uint32_t num_elems_per_reg_ = 0;
        uint32_t num_elems_per_uop_ = 0;
        uint32_t num_uops_to_sequence_ = 0;
        uint32_t num_uops_sequenced_ = 0;

        // State for vector permutes
        uint32_t num_permute_srcs_to_load_ = 0;
        uint32_t num_permute_srcs_loaded_ = 0;
        uint32_t num_permute_exe_uops_to_sequence_ = 0;
        uint32_t num_permute_exe_uops_sequenced_ = 0;

        // State for indexed load/store sequencing
        uint32_t num_elems_sequenced_ = 0;
        uint32_t num_indexes_per_uop_ = 0;

        // Instruction sequencer functions
        template <InstArchInfo::UopGenType Type> MavisOperands sequenceUops_();

        template <InstArchInfo::UopGenType Type> MavisOperands sequenceSlideOneUops_();

        template <InstArchInfo::UopGenType Type> MavisOperands sequencePermuteUops_();

        MavisOperands sequenceHawkPermuteUops_();

        MavisOperands sequenceIndexLdstUops_();

        void reset_()
        {
            current_inst_.clearValid();
            current_inst_modifiers_.clear();
            num_elems_per_reg_ = 0;
            num_elems_per_uop_ = 0;
            num_uops_to_sequence_ = 0;
            num_uops_sequenced_ = 0;
            num_permute_srcs_to_load_ = 0;
            num_permute_srcs_loaded_ = 0;
            num_permute_exe_uops_to_sequence_ = 0;
            num_permute_exe_uops_sequenced_ = 0;
            num_elems_sequenced_ = 0;
            num_indexes_per_uop_ = 0;
        }

        void addModifier(const std::string & name, uint32_t value)
        {
            current_inst_modifiers_.emplace_back(name, value);
        }

        std::optional<uint32_t> getModifier(const std::string & name)
        {
            for (auto & m : current_inst_modifiers_)
            {
                if (m.getName() == name)
                {
                    return m.getValue();
                }
            }
            return {};
        }
    };

    class VectorSequencerTester;
} // namespace olympia
