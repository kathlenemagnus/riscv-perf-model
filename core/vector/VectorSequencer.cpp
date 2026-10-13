#include "vector/VectorSequencer.hpp"
#include "Inst.hpp"
#include "mavis/Mavis.h"
#include "mavis/InstMetaData.h"
#include "sparta/utils/LogUtils.hpp"

namespace olympia
{
    VectorSequencer::VectorSequencer(const uint32_t vlen, const uint32_t vlen_dlen_ratio,
                                     const bool sequence_tail_uops) :
        vlen_(vlen),
        vlen_dlen_ratio_(vlen_dlen_ratio),
        sequence_tail_uops_(sequence_tail_uops)
    {
        sparta_assert(vlen_dlen_ratio_ == 1 || vlen_dlen_ratio_ == 2,
                      "Unsupported VLEN to DLEN ratio: " << vlen_dlen_ratio_);

        // Vector elementwise uop generator, increment all src and dest register numbers
        // For a "vadd.vv v12, v4,v8" with an LMUL of 4:
        //    Uop 1: vadd.vv v12, v4, v8
        //    Uop 2: vadd.vv v13, v5, v9
        //    Uop 3: vadd.vv v14, v6, v10
        //    Uop 4: vadd.vv v15, v7, v11
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::ELEMENTWISE,
            &VectorSequencer::sequenceUops_<InstArchInfo::UopGenType::ELEMENTWISE>);

        // Vector single dest uop generator, only increment all src register numbers
        // For a "vmseq.vv v12, v4, v8" with an LMUL of 4:
        //    Uop 1: vmseq.vv v12, v4, v8
        //    Uop 2: vmseq.vv v12, v5, v9
        //    Uop 3: vmseq.vv v12, v6, v10
        //    Uop 4: vmseq.vv v12, v7, v11
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::SINGLE_DEST,
            &VectorSequencer::sequenceUops_<InstArchInfo::UopGenType::SINGLE_DEST>);

        // Vector single src uop generator, only increment dst register numbers
        // For a "viota.m v0, v8" with an LMUL of 4:
        //    Uop 1: viota.m v0, v8
        //    Uop 2: viota.m v0, v9
        //    Uop 3: viota.m v0, v10
        //    Uop 4: viota.m v0, v11
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::SINGLE_SRC,
            &VectorSequencer::sequenceUops_<InstArchInfo::UopGenType::SINGLE_SRC>);

        // Vector wide uop generator, only increment src register numbers for even
        // uops
        // For a "vwmul.vv v12, v4, v8" with an LMUL of 4:
        //    Uop 1: vwmul.vv v12, v4, v8
        //    Uop 2: vwmul.vv v13, v4, v8
        //    Uop 3: vwmul.vv v14, v6, v10
        //    Uop 4: vwmul.vv v15, v6, v10
        //    Uop 5: vwmul.vv v16, v8, v12
        //    Uop 6: vwmul.vv v17, v8, v12
        //    Uop 7: vwmul.vv v18, v10, v14
        //    Uop 8: vwmul.vv v19, v10, v14
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::WIDENING,
            &VectorSequencer::sequenceUops_<InstArchInfo::UopGenType::WIDENING>);

        // Vector wide mixed uop generator
        // For a "vwaddu.wv v12, v4, v8" with an LMUL of 4:
        //    Uop 1: vwaddu.wv v12, v4, v8
        //    Uop 2: vwaddu.wv v13, v5, v8
        //    Uop 3: vwaddu.wv v14, v6, v10
        //    Uop 4: vwaddu.wv v15, v7, v10
        //    Uop 5: vwaddu.wv v16, v8, v12
        //    Uop 6: vwaddu.wv v17, v9, v12
        //    Uop 7: vwaddu.wv v18, v10, v14
        //    Uop 8: vwaddu.wv v19, v11, v14
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::WIDENING_MIXED,
            &VectorSequencer::sequenceUops_<InstArchInfo::UopGenType::WIDENING_MIXED>);

        // Vector arithmetic multiply-add uop generator, add dest as source
        // For a "vmacc.vv v12, v4, v8" with an LMUL of 4:
        //    Uop 1: vmacc.vv v12, v4, v8, v12
        //    Uop 2: vmacc.vv v13, v5, v9, v13
        //    Uop 3: vmacc.vv v14, v6, v10, v14
        //    Uop 4: vmacc.vv v15, v7, v11, v15
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::MAC,
            &VectorSequencer::sequenceUops_<InstArchInfo::UopGenType::MAC>);

        // Vector multiply-add wide dest uop generator
        // For a "vwmacc.vv v12, v4, v8" with an LMUL of 4:
        //    Uop 1: vwmacc.vv v12, v4, v8, v12
        //    Uop 2: vwmacc.vv v13, v4, v8, v13
        //    Uop 3: vwmacc.vv v14, v5, v9, v14
        //    Uop 4: vwmacc.vv v15, v5, v9, v15
        //    Uop 5: vwmacc.vv v16, v6, v10, v16
        //    Uop 6: vwmacc.vv v17, v6, v10, v17
        //    Uop 7: vwmacc.vv v18, v7, v11, v18
        //    Uop 8: vwmacc.vv v19, v7, v11, v19
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::MAC_WIDE,
            &VectorSequencer::sequenceUops_<InstArchInfo::UopGenType::MAC_WIDE>);

        // Vector fixed point clip narrow uop generator
        // For a "vnclipu.wv v0, v4, v8" with an LMUL of 4:
        //    Uop 1: vnclipu.wv v0, v4, v5, v12
        //    Uop 2: vnclipu.wv v1, v6, v7, v13
        //    Uop 3: vnclipu.wv v2, v8, v9, v14
        //    Uop 4: vnclipu.wv v3, v10, v11, v15
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::NARROWING,
            &VectorSequencer::sequenceUops_<InstArchInfo::UopGenType::NARROWING>);

        // Vector reduction uop generator
        // For a "vredsum.vs v12, v8, v4" with an LMUL of 4:
        //    Uop 1: vredsum.vs v12, v8, v4
        //    Uop 2: vredsum.vs v13, v9, v5, v12
        //    Uop 3: vredsum.vs v14, v10, v6, v13
        //    Uop 4: vredsum.vs v15, v11, v7, v14
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::RED,
            &VectorSequencer::sequenceUops_<InstArchInfo::UopGenType::RED>);

        // Vector wide reduction uop generator
        // For a "vwredsum.vs v20, v12, v4" with an LMUL of 4:
        //    Uop 1: vredsum.vs v20, v12, v4
        //    Uop 2: vredsum.vs v21, v12, v5, v20
        //    Uop 3: vredsum.vs v22, v13, v6, v21
        //    Uop 4: vredsum.vs v23, v13, v7, v22
        //    Uop 5: vredsum.vs v24, v14, v8, v23
        //    Uop 6: vredsum.vs v25, v14, v9, v24
        //    Uop 7: vredsum.vs v26, v15, v10, v25
        //    Uop 8: vredsum.vs v27, v15, v11, v26
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::RED_WIDE,
            &VectorSequencer::sequenceUops_<InstArchInfo::UopGenType::RED_WIDE>);

        // Vector fixed point clip narrow uop generator
        // For a "vzext.vf4 v0, v4" with an LMUL of 4:
        //    Uop 1: vzext.vf4 v0, v4
        //    Uop 2: vzext.vf4 v1, v4
        //    Uop 3: vzext.vf4 v2, v4
        //    Uop 4: vzext.vf4 v3, v4
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::INT_EXT,
            &VectorSequencer::sequenceUops_<InstArchInfo::UopGenType::INT_EXT>);

        // Vector slide 1 up uop generator
        // For a "vslide1up.vx v4, v8, x1" with an LMUL of 4:
        //    Uop 1: vslide1up.vx v4, v8, x1
        //    Uop 2: vslide1up.vx v4, v9, v8
        //    Uop 3: vslide1up.vx v4, v10, v9
        //    Uop 4: vslide1up.vx v4, v11, v10
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::SLIDE1UP,
            &VectorSequencer::sequenceSlideOneUops_<InstArchInfo::UopGenType::SLIDE1UP>);

        // Vector slide 1 down uop generator
        // For a "vslide1ddown.vx v4, v8, x1" with an LMUL of 4:
        //    Uop 1: vslide1down.vx v4, v8, v9
        //    Uop 2: vslide1down.vx v4, v9, v10
        //    Uop 3: vslide1down.vx v4, v10, v11
        //    Uop 4: vslide1down.vx v4, v11, x1
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::SLIDE1DOWN,
            &VectorSequencer::sequenceSlideOneUops_<InstArchInfo::UopGenType::SLIDE1DOWN>);

        // Vector slide uop generator
        // For a "vslideup.vx v20, v8, x1" with an LMUL of 4:
        //    Load Uop 1: vslideup.vx v8, v9
        //    Load Uop 1: vslideup.vx v10, v11
        //     Exe Uop 1: vslideup.vx v20
        //     Exe Uop 2: vslideup.vx v21
        //     Exe Uop 3: vslideup.vx v22
        //     Exe Uop 4: vslideup.vx v23
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::SLIDE,
            &VectorSequencer::sequencePermuteUops_<InstArchInfo::UopGenType::SLIDE>);

        // Vector permute uop generator
        // For a "vrgather.vv v20, v8, v4" with an LMUL of 4:
        //    Load Uop 1: vrgather.vv v4, v5
        //    Load Uop 1: vrgather.vv v6, v7
        //     Exe Uop 1: vrgather.vv v20, v8
        //     Exe Uop 2: vrgather.vv v21, v9
        //     Exe Uop 3: vrgather.vv v22, v10
        //     Exe Uop 4: vrgather.vv v23, v11
        uop_gen_function_map_.emplace(
            InstArchInfo::UopGenType::PERMUTE,
            &VectorSequencer::sequencePermuteUops_<InstArchInfo::UopGenType::PERMUTE>);

        // Vector indexed load/store uop generator
        // Increment all store data src and load dest register numbers based on SEW and LMUL
        // Increment index src register number based on EEW and EMUL
        // For a "vloxei64.v v2, (rs1),v4" with VLEN=128, SEW=32, LMUL=2:
        //    Uop 1: vloxei64.v v2, (rs1), v4     -- also needs v5 as a src
        //    Uop 2: vloxei64.v v3, (rs1), v6     -- also needs v7 as a src
        uop_gen_function_map_.emplace(InstArchInfo::UopGenType::INDEX_LDST,
                                      &VectorSequencer::sequenceIndexLdstUops_);
    }

    std::vector<VectorSequencer::UopInfo> VectorSequencer::sequenceInst(const InstPtr & inst)
    {
        const auto uop_gen_type = inst->getUopGenType();
        sparta_assert(uop_gen_type != InstArchInfo::UopGenType::UNKNOWN,
                      "Inst: " << inst << " uop gen type is unknown");
        sparta_assert(uop_gen_type != InstArchInfo::UopGenType::NONE,
                      "Inst: " << inst << " uop gen type is none");

        const auto mavis_uid = inst->getUniqueID();
        if (uop_gen_type == InstArchInfo::UopGenType::INT_EXT)
        {
            if ((mavis_uid == MAVIS_UID_VZEXTVF2) || (mavis_uid == MAVIS_UID_VSEXTVF2))
            {
                addModifier("viext", 2);
            }
            else if ((mavis_uid == MAVIS_UID_VZEXTVF4) || (mavis_uid == MAVIS_UID_VSEXTVF4))
            {
                addModifier("viext", 4);
            }
            else if ((mavis_uid == MAVIS_UID_VZEXTVF8) || (mavis_uid == MAVIS_UID_VSEXTVF8))
            {
                addModifier("viext", 8);
            }
        }

        // Number of vector elements processed by each uop
        // For unit-stride and strided load/store, this depends on EEW instead of SEW
        // EEW comes from the mavis encoding in the instruction
        const vector_types::VectorGlobalConfig* vec_global_config = inst->getVectorGlobalConfig();
        const uint32_t sew = vec_global_config->getSEW();
        const uint32_t eew = inst->getEEW();
        const bool use_eew =
            inst->isLoadStoreInst() && (uop_gen_type != InstArchInfo::UopGenType::INDEX_LDST);
        const uint32_t elem_width = use_eew ? eew : sew;
        // Vector length of the current inst, sometimes overridden by whole register insts
        uint64_t vl = vec_global_config->getVL();

        // How many elements are contained in each vector register
        num_elems_per_reg_ = vlen_ / elem_width;
        // How many elements can be processed per uop, it may be less than num_elems_per_reg_
        // for some instructions
        num_elems_per_uop_ = num_elems_per_reg_ / vlen_dlen_ratio_;

        if ((uop_gen_type == InstArchInfo::UopGenType::INDEX_LDST)
            || (mavis_uid == MAVIS_UID_VRGATHEREI16))
        {
            num_indexes_per_uop_ = (vlen_ / eew) / vlen_dlen_ratio_;
            num_elems_per_uop_ = std::min(num_elems_per_uop_, num_indexes_per_uop_);
        }

        if (sequence_tail_uops_)
        {
            num_uops_to_sequence_ =
                std::ceil((float)vec_global_config->getVLMAX() / num_elems_per_uop_);
        }
        else
        {
            num_uops_to_sequence_ = std::ceil((float)vl / num_elems_per_uop_);
            // Round VL to a register boundary so all DLEN idxs are sequenced
            if (auto rem = num_uops_to_sequence_ % vlen_dlen_ratio_; rem != 0)
            {
                num_uops_to_sequence_ = num_uops_to_sequence_ + vlen_dlen_ratio_ - rem;
            }
        }

        if ((uop_gen_type == InstArchInfo::UopGenType::WIDENING)
            || (uop_gen_type == InstArchInfo::UopGenType::WIDENING_MIXED)
            || (uop_gen_type == InstArchInfo::UopGenType::MAC_WIDE)
            || (uop_gen_type == InstArchInfo::UopGenType::RED_WIDE))
        {
            sparta_assert(vec_global_config->getLMUL() <= 4,
                          "LMUL must be lower or equal to 4.\n"
                          "These modes set EMUL = 2 * LMUL <= 8.");
            // TODO: Add parameter to support dual dests
            num_uops_to_sequence_ *= 2;
        }
        else if (inst->isVectorWholeRegister())
        {
            // For vector move, load and store whole register instructions
            if (mavis_uid == MAVIS_UID_VMV1R)
            {
                num_uops_to_sequence_ = 1;
            }
            else if (mavis_uid == MAVIS_UID_VMV2R)
            {
                num_uops_to_sequence_ = 2;
            }
            else if (mavis_uid == MAVIS_UID_VMV4R)
            {
                num_uops_to_sequence_ = 4;
            }
            else if (mavis_uid == MAVIS_UID_VMV8R)
            {
                num_uops_to_sequence_ = 8;
            }
            else
            {
                // NFIELDS encoding: 0x000=1, 0x001=2, 0x010=3, etc...
                num_uops_to_sequence_ =
                    (inst->getSpecialField(mavis::OpcodeInfo::SpecialField::NF) + 1);
            }

            num_uops_to_sequence_ *= vlen_dlen_ratio_;
            vl = num_uops_to_sequence_ * num_elems_per_uop_;
        }
        else if ((mavis_uid == MAVIS_UID_VLM) || (mavis_uid == MAVIS_UID_VSM))
        {
            // For vector mask load and store instructions
            num_uops_to_sequence_ = vlen_dlen_ratio_;
            vl = std::ceil((float)vl / 8);
        }
        else if (inst->isVector() && inst->isLoadStoreInst())
        {
            // Take NFIELDS into account.
            // For regular vector load/store NFIELDS will be 1, so the following won't actually
            // change num_uops_to_sequence_, but for segmented load/store, need to multiply
            // by NFIELDS to get correct number of uops
            // Per the RISC-V spec, ceil(EMUL) * NFIELDS ≤ 8 or the encoding is reserved
            const uint32_t nfields = inst->getNfields();
            sparta_assert(nfields > 0,
                          "Invalid value of nfields (" << nfields << ") for inst: " << inst);
            const uint32_t emul = std::ceil(inst->getEMUL());
            sparta_assert((emul * nfields) <= 8,
                          "Invalid combination of EMUL and NFIELDS. ceil(emul)="
                              << emul << ", NFIELDS=" << nfields << " inst: " << inst);
            // Sanity check the size of max_num_uops_to_sequence before multiplying.
            // The max possible value it can have at this point based on code above
            // is 8 * vlen_dlen_ratio_, but if it is a segmented instruction, it will
            // actually be less than that (otherwise nfields could only be 1)
            const uint32_t max_num_uops_to_sequence = 8 * vlen_dlen_ratio_;
            sparta_assert(num_uops_to_sequence_ <= max_num_uops_to_sequence,
                          "Cannot sequence more than " << max_num_uops_to_sequence
                                                       << " vector uops: " << inst);

            num_uops_to_sequence_ *= nfields;
            // vl elements should be operated on per field
            vl *= nfields;
        }

        if ((uop_gen_type == InstArchInfo::UopGenType::SLIDE)
            || (uop_gen_type == InstArchInfo::UopGenType::PERMUTE))
        {
            // Can load 2 srcs per load uop
            constexpr uint32_t NUM_SRCS_PER_PERMUTE_LOAD_UOP = 2;
            num_permute_srcs_to_load_ = std::ceil((float)vl / num_elems_per_reg_);
            uint32_t num_permute_load_uops_to_sequence =
                std::ceil((float)num_permute_srcs_to_load_ / NUM_SRCS_PER_PERMUTE_LOAD_UOP);

            // Can write 1 dest per exe uop
            num_permute_exe_uops_to_sequence_ = num_permute_srcs_to_load_;

            num_permute_load_uops_to_sequence *= vlen_dlen_ratio_;
            num_permute_exe_uops_to_sequence_ *= vlen_dlen_ratio_;
            num_uops_to_sequence_ =
                num_permute_load_uops_to_sequence + num_permute_exe_uops_to_sequence_;

            const uint32_t max_permute_ops_to_sequence = 12 * vlen_dlen_ratio_;
            sparta_assert(num_uops_to_sequence_ <= max_permute_ops_to_sequence,
                          "Cannot sequence more than " << max_permute_ops_to_sequence
                                                       << " vector permute uops: " << inst);
        }
        else
        {
            const uint32_t max_num_uops_to_sequence = 8 * vlen_dlen_ratio_;
            sparta_assert(num_uops_to_sequence_ <= max_num_uops_to_sequence,
                          "Cannot sequence more than " << max_num_uops_to_sequence
                                                       << " vector uops: " << inst);
        }

        // Sequence inst
        current_inst_ = inst;
        std::vector<UopInfo> uops;
        auto uop_gen_func = uop_gen_function_map_.at(uop_gen_type);

        uint32_t dlen_idx = 0;
        while (num_uops_sequenced_ != num_uops_to_sequence_)
        {
            // Generate uop, get source and dest operands
            const MavisOperands mavis_operands = uop_gen_func(this);

            // Calculate the starting element number for this uop. Will be used for load/store
            // uops to index into the vector of memory addresses to get the correct address
            // for each element
            const uint64_t start_elem_this_uop = num_elems_per_uop_ * num_uops_sequenced_;

            // Bit mask to track which elements are active
            // FIXME: This is broken for permutes
            const bool has_tail = (start_elem_this_uop + num_elems_per_uop_) > vl;
            const uint64_t num_elems_this_uop = !has_tail ? num_elems_per_uop_
                                                : (vl < start_elem_this_uop)
                                                    ? 0
                                                    : (vl - start_elem_this_uop);
            boost::dynamic_bitset<> active_elem_mask(
                num_elems_per_uop_); // 1=active, 0=inactive/masked/tail
            active_elem_mask.set();  // sets all bits
            if (SPARTA_EXPECT_FALSE(has_tail))
            {
                for (uint32_t bit_idx = num_elems_this_uop; bit_idx < num_elems_per_uop_; ++bit_idx)
                {
                    active_elem_mask.reset(bit_idx);
                }
            }

            /*
            if (SPARTA_EXPECT_FALSE(inst->getVectorMaskEnabled()))
            {
                const auto & mask = inst->getVectorMask();
                for (uint32_t bit_idx = 0; bit_idx < num_elems_this_uop; ++bit_idx)
                {
                    active_elem_mask[bit_idx] &= mask[start_elem_this_uop + bit_idx];
                }
            }
            */

            uops.emplace_back(mavis_operands.first, mavis_operands.second, start_elem_this_uop,
                              active_elem_mask, dlen_idx);

            ++num_uops_sequenced_;

            // If VLEN > DLEN then a vector uop will only operate on a portion of its vector
            // registers
            if (vlen_dlen_ratio_ > 1)
            {
                dlen_idx = dlen_idx == (vlen_dlen_ratio_ - 1) ? 0 : ++dlen_idx;
            }
        }

        reset_();
        return uops;
    }

    template <InstArchInfo::UopGenType Type>
    VectorSequencer::MavisOperands VectorSequencer::sequenceUops_()
    {
        sparta_assert(current_inst_.isValid(),
                      "Cannot sequence uops, current instruction is not set");

        mavis::OperandInfo::ElementList srcs;
        mavis::OperandInfo::Element src_rs3;
        for (auto src : current_inst_.getValue()->getSourceOpInfoList())
        {
            if (src.operand_type != mavis::InstMetaData::OperandTypes::VECTOR
                || Type == InstArchInfo::UopGenType::SINGLE_SRC)
            {
                srcs.emplace_back(src);
                continue;
            }

            if constexpr (Type == InstArchInfo::UopGenType::ELEMENTWISE
                          || Type == InstArchInfo::UopGenType::MAC
                          || Type == InstArchInfo::UopGenType::SINGLE_DEST)
            {
                src.field_value += num_uops_sequenced_ / vlen_dlen_ratio_;
            }
            else if constexpr (Type == InstArchInfo::UopGenType::WIDENING)
            {
                src.field_value += num_uops_sequenced_ / 2 / vlen_dlen_ratio_;
            }
            else if constexpr (Type == InstArchInfo::UopGenType::MAC_WIDE)
            {
                if (src.field_id == mavis::InstMetaData::OperandFieldID::RD)
                {
                    src.field_value += num_uops_sequenced_ / vlen_dlen_ratio_;
                }
                else
                {
                    src.field_value += num_uops_sequenced_ / 2 / vlen_dlen_ratio_;
                }
            }
            else if constexpr (Type == InstArchInfo::UopGenType::WIDENING_MIXED)
            {
                if (src.field_id == mavis::InstMetaData::OperandFieldID::RS2)
                {
                    src.field_value += num_uops_sequenced_ / vlen_dlen_ratio_;
                }
                else if (src.field_id == mavis::InstMetaData::OperandFieldID::RS1)
                {
                    src.field_value += num_uops_sequenced_ / 2 / vlen_dlen_ratio_;
                }
            }
            else if constexpr (Type == InstArchInfo::UopGenType::NARROWING)
            {
                const vector_types::VectorGlobalConfig* vec_global_config =
                    current_inst_.getValue()->getVectorGlobalConfig();
                if ((src.field_id == mavis::InstMetaData::OperandFieldID::RS2)
                    && (vec_global_config->getLMUL() >= 1))
                {
                    src_rs3 = src;
                    src.field_value += num_uops_sequenced_ / vlen_dlen_ratio_;
                    src_rs3.field_value = src.field_value + 1;
                }
                else if (src.field_id == mavis::InstMetaData::OperandFieldID::RS1)
                {
                    src.field_value += num_uops_sequenced_ / vlen_dlen_ratio_;
                }
            }
            else if constexpr ((Type == InstArchInfo::UopGenType::RED)
                               || (Type == InstArchInfo::UopGenType::RED_WIDE))
            {
                if (src.field_id == mavis::InstMetaData::OperandFieldID::RS2)
                {
                    src.field_value += num_uops_sequenced_ / vlen_dlen_ratio_;
                }
            }
            else if constexpr (Type == InstArchInfo::UopGenType::INT_EXT)
            {
                auto ext = getModifier("viext");
                if (!ext)
                {
                    throw sparta::SpartaException("Modifier at current instruction doesnt exist.");
                }
                src.field_value += num_uops_sequenced_ / *ext / vlen_dlen_ratio_;
            }

            srcs.emplace_back(src);
        }

        // For narrowing insturction,
        if constexpr (Type == InstArchInfo::UopGenType::NARROWING)
        {
            // Narrowing instructions only need to add an RS3 when LMUL >= 1 (not fractional)
            if (src_rs3.field_id != mavis::InstMetaData::OperandFieldID::NONE)
            {
                srcs.emplace_back(mavis::InstMetaData::OperandFieldID::RS3, src_rs3.operand_type,
                                  src_rs3.field_value);
            }
        }

        // Add a destination to the list of sources
        auto add_dest_as_src = [](auto & srcs, auto & dest)
        {
            // OperandFieldID is an enum with RS1 = 0, RS2 = 1, etc. with a max RS of RS4
            using OperandFieldID = mavis::InstMetaData::OperandFieldID;
            const OperandFieldID field_id = static_cast<OperandFieldID>(srcs.size());
            sparta_assert(
                field_id <= OperandFieldID::RS_MAX,
                "Mavis does not support instructions with more than "
                    << std::dec
                    << static_cast<std::underlying_type_t<OperandFieldID>>(OperandFieldID::RS_MAX)
                    << " sources");
            srcs.emplace_back(field_id, dest.operand_type, dest.field_value);
        };

        mavis::OperandInfo::ElementList dests = current_inst_.getValue()->getDestOpInfoList();
        constexpr bool single_dest = (Type == InstArchInfo::UopGenType::SINGLE_DEST)
                                     || (Type == InstArchInfo::UopGenType::RED)
                                     || (Type == InstArchInfo::UopGenType::RED_WIDE);
        for (auto & dest : dests)
        {
            if constexpr (!single_dest)
            {
                dest.field_value += num_uops_sequenced_ / vlen_dlen_ratio_;
            }

            if constexpr ((Type == InstArchInfo::UopGenType::RED)
                          || (Type == InstArchInfo::UopGenType::RED_WIDE))
            {
                if (num_uops_sequenced_ != 0)
                {
                    add_dest_as_src(srcs, dest);
                }
            }
        }

        return std::make_pair(srcs, dests);
    }

    template <InstArchInfo::UopGenType Type>
    VectorSequencer::MavisOperands VectorSequencer::sequenceSlideOneUops_()
    {
        static_assert((Type == InstArchInfo::UopGenType::SLIDE1UP)
                      || (Type == InstArchInfo::UopGenType::SLIDE1DOWN));
        sparta_assert(current_inst_.isValid(),
                      "Cannot sequence uops, current instruction is not set");
        auto orig_srcs = current_inst_.getValue()->getSourceOpInfoList();

        mavis::OperandInfo::ElementList srcs;
        for (auto & src : orig_srcs)
        {
            if constexpr (Type == InstArchInfo::UopGenType::SLIDE1UP)
            {
                // For vslide1up, first uop sources the scalar register
                if (src.operand_type != mavis::InstMetaData::OperandTypes::VECTOR)
                {
                    if ((num_uops_sequenced_ / vlen_dlen_ratio_) == 0)
                    {
                        srcs.emplace_back(src);
                    }
                }
                else
                {
                    srcs.emplace_back(src.field_id, src.operand_type,
                                      src.field_value + (num_uops_sequenced_ / vlen_dlen_ratio_));
                    if ((num_uops_sequenced_ / vlen_dlen_ratio_) != 0)
                    {
                        using OperandFieldID = mavis::InstMetaData::OperandFieldID;
                        const OperandFieldID field_id = OperandFieldID::RS3;
                        using OperandFieldValue = mavis::OperandInfo::OpcodeFieldValueType;
                        const OperandFieldValue field_value =
                            src.field_value + (num_uops_sequenced_ / vlen_dlen_ratio_) - 1;
                        srcs.emplace_back(field_id, src.operand_type, field_value);
                    }
                }
            }
            else if constexpr (Type == InstArchInfo::UopGenType::SLIDE1DOWN)
            {
                // For vslide1down, last uop sources the scalar register
                if (src.operand_type != mavis::InstMetaData::OperandTypes::VECTOR)
                {
                    if (((num_uops_sequenced_ / vlen_dlen_ratio_) + 1) == num_uops_to_sequence_)
                    {
                        srcs.emplace_back(src);
                    }
                }
                else
                {
                    srcs.emplace_back(src.field_id, src.operand_type,
                                      src.field_value + (num_uops_sequenced_ / vlen_dlen_ratio_));
                    if (((num_uops_sequenced_ / vlen_dlen_ratio_) + 1) != num_uops_to_sequence_)
                    {
                        using OperandFieldID = mavis::InstMetaData::OperandFieldID;
                        const OperandFieldID field_id = OperandFieldID::RS3;
                        using OperandFieldValue = mavis::OperandInfo::OpcodeFieldValueType;
                        const OperandFieldValue field_value =
                            src.field_value + (num_uops_sequenced_ / vlen_dlen_ratio_) + 1;
                        srcs.emplace_back(field_id, src.operand_type, field_value);
                    }
                }
            }
        }

        mavis::OperandInfo::ElementList dests = current_inst_.getValue()->getDestOpInfoList();
        for (auto & dest : dests)
        {
            dest.field_value += num_uops_sequenced_ / vlen_dlen_ratio_;
        }

        return std::make_pair(srcs, dests);
    }

    template <InstArchInfo::UopGenType Type>
    VectorSequencer::MavisOperands VectorSequencer::sequencePermuteUops_()
    {
        sparta_assert(current_inst_.isValid(),
                      "Cannot sequence uops, current instruction is not set");
        const auto orig_srcs = current_inst_.getValue()->getSourceOpInfoList();
        const auto orig_dests = current_inst_.getValue()->getDestOpInfoList();

        mavis::OperandInfo::ElementList srcs;
        mavis::OperandInfo::ElementList dests;

        // Generate load uops
        if (num_permute_srcs_loaded_ < num_permute_srcs_to_load_)
        {
            const auto src_rs2_it = std::find_if(
                orig_srcs.begin(), orig_srcs.end(), [](const mavis::OperandInfo::Element & src)
                { return src.field_id == mavis::InstMetaData::OperandFieldID::RS2; });
            sparta_assert(
                src_rs2_it != orig_srcs.end(),
                "Failed to find VS2 for vector permute inst: " << current_inst_.getValue());
            const auto src = *src_rs2_it;

            // TODO: Current design allows 2 sources to be loaded per uop, needs parameter
            srcs.emplace_back(src.field_id, src.operand_type,
                              src.field_value + num_permute_srcs_loaded_);
            if ((num_permute_srcs_loaded_ + 1) < num_permute_srcs_to_load_)
            {
                using OperandFieldID = mavis::InstMetaData::OperandFieldID;
                const OperandFieldID field_id = OperandFieldID::RS3;
                srcs.emplace_back(field_id, src.operand_type,
                                  src.field_value + num_permute_srcs_loaded_ + 1);
                if (((num_uops_sequenced_ + 1) % vlen_dlen_ratio_) == 0)
                {
                    ++num_permute_srcs_loaded_;
                }
            }
            if (((num_uops_sequenced_ + 1) % vlen_dlen_ratio_) == 0)
            {
                ++num_permute_srcs_loaded_;
            }
        }
        // Generate execution uops
        else
        {
            if constexpr (Type == InstArchInfo::UopGenType::PERMUTE)
            {
                const auto inst_ptr = current_inst_.getValue();
                if (inst_ptr->hasImmediate() == false)
                {
                    const auto src_rs1_it = std::find_if(
                        orig_srcs.begin(), orig_srcs.end(),
                        [](const mavis::OperandInfo::Element & src)
                        { return src.field_id == mavis::InstMetaData::OperandFieldID::RS1; });
                    sparta_assert(
                        src_rs1_it != orig_srcs.end(),
                        "Failed to find VS1 for vector permute inst: " << current_inst_.getValue());
                    const auto src = *src_rs1_it;

                    using OperandFieldValue = mavis::OperandInfo::OpcodeFieldValueType;
                    OperandFieldValue field_value = src.field_value;
                    if (src.operand_type == mavis::InstMetaData::OperandTypes::VECTOR)
                    {
                        if (inst_ptr->getUniqueID() == MAVIS_UID_VRGATHEREI16)
                        {
                            field_value += ((float)num_elems_sequenced_ / num_indexes_per_uop_)
                                           / vlen_dlen_ratio_;
                            num_elems_sequenced_ += num_elems_per_uop_;
                        }
                        else
                        {
                            field_value += (num_permute_exe_uops_sequenced_ / vlen_dlen_ratio_);
                        }
                    }
                    srcs.emplace_back(src.field_id, src.operand_type, field_value);
                }
            }

            for (auto & dest : orig_dests)
            {
                dests.emplace_back(dest.field_id, dest.operand_type,
                                   dest.field_value
                                       + (num_permute_exe_uops_sequenced_ / vlen_dlen_ratio_));
            }

            ++num_permute_exe_uops_sequenced_;
        }

        return std::make_pair(srcs, dests);
    }

    VectorSequencer::MavisOperands VectorSequencer::sequenceHawkPermuteUops_()
    {
        sparta_assert(current_inst_.isValid(),
                      "Cannot sequence uops, current instruction is not set");

        using OperandFieldID = mavis::InstMetaData::OperandFieldID;
        const auto inst = current_inst_.getValue();
        mavis::OperandInfo::ElementList srcs = inst->getSourceOpInfoList();
        for (auto & src : srcs)
        {
            if (src.operand_type != mavis::InstMetaData::OperandTypes::VECTOR)
            {
                // scalar sources (eg. base addr) are left alone
                continue;
            }
            else if ((inst->getUniqueID() != MAVIS_UID_VRGATHEREI16)
                     || (src.field_id == mavis::InstMetaData::OperandFieldID::RS2))
            {
                src.field_value += num_elems_sequenced_ / num_elems_per_reg_;
            }
            else if (src.field_id == mavis::InstMetaData::OperandFieldID::RS1)
            {
                sparta_assert(
                    src.field_id == mavis::InstMetaData::OperandFieldID::RS1,
                    "What is this if not RS1? "
                        << static_cast<std::underlying_type_t<OperandFieldID>>(src.field_id));
                src.field_value += num_elems_sequenced_ / num_indexes_per_uop_ / vlen_dlen_ratio_;
            }
        }

        mavis::OperandInfo::ElementList dests = inst->getDestOpInfoList();
        for (auto & dest : dests)
        {
            // Dest register is incremented for each uop
            dest.field_value += num_elems_sequenced_ / num_elems_per_reg_;
        }

        num_elems_sequenced_ += num_elems_per_uop_;
        return std::make_pair(srcs, dests);
    }

    VectorSequencer::MavisOperands VectorSequencer::sequenceIndexLdstUops_()
    {
        sparta_assert(current_inst_.isValid(),
                      "Cannot sequence uops, current instruction is not set");

        using OperandFieldID = mavis::InstMetaData::OperandFieldID;
        const auto inst = current_inst_.getValue();
        mavis::OperandInfo::ElementList srcs = inst->getSourceOpInfoList();
        for (auto & src : srcs)
        {
            if (src.operand_type != mavis::InstMetaData::OperandTypes::VECTOR)
            {
                // scalar sources (eg. base addr) are left alone
                continue;
            }
            else if (src.field_id == mavis::InstMetaData::OperandFieldID::RS3)
            {
                // Store data register is incremented for each uop
                src.field_value += num_elems_sequenced_ / num_elems_per_reg_;
            }
            else
            {
                sparta_assert(
                    src.field_id == mavis::InstMetaData::OperandFieldID::RS2,
                    "What is this if not RS2? "
                        << static_cast<std::underlying_type_t<OperandFieldID>>(src.field_id));

                src.field_value += num_elems_sequenced_ / num_indexes_per_uop_ / vlen_dlen_ratio_;
            }
        }

        mavis::OperandInfo::ElementList dests = inst->getDestOpInfoList();
        for (auto & dest : dests)
        {
            // Dest register is incremented for each uop
            dest.field_value += num_elems_sequenced_ / num_elems_per_reg_;
        }

        num_elems_sequenced_ += num_elems_per_uop_;
        return std::make_pair(srcs, dests);
    }
} // namespace olympia
