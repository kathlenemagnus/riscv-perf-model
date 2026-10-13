// <VectorTypes.hpp> -*- C++ -*-

#pragma once

#include <set>
#include <boost/dynamic_bitset.hpp>

#include "vector/VectorUtils.hpp"
#include "sparta/utils/SpartaSharedPointer.hpp"

namespace olympia::vector_types
{
    // Maximum vector length supported by olympia
    static constexpr uint32_t MAX_VLEN_SUPPORTED = 1024;

    // Maximum number of elements per vector uop (or vector register)
    // Worst case is when element size is 8, which means VLEN/8 elements
    static constexpr uint32_t MAX_ELEM_PER_VECTOR_OP = MAX_VLEN_SUPPORTED / 8;

    /*!
     * \class VectorGlobalConfig
     * \brief Contains global vector configuration
     */
    class VectorGlobalConfig
    {
      public:
        VectorGlobalConfig(const uint32_t vlen, const uint32_t vl, const uint32_t sew,
                           const float lmul, const bool vta, const bool vma) :
            vlen_(vlen),
            sew_(sew),
            lmul_(lmul),
            vlmax_(vector_utils::getVlmax(vlen_, sew_, lmul_)),
            vl_(vl),
            vta_(vta),
            vma_(vma)
        {
            sparta_assert(vector_utils::isVlenValid(vlen_), "Invalid VLEN: " << vlen_);
            sparta_assert(vector_utils::isSewValid(sew_), "Invalid SEW: " << sew_);
            sparta_assert(vector_utils::isLmulValid(lmul_), "Invalid LMUL: " << lmul_);
            sparta_assert(vl_ <= vlmax_,
                          "VL (" << vl_ << ") cannot be greater than VLMAX (" << vlmax_ << ")");
        }

        VectorGlobalConfig(const VectorGlobalConfig &) = default;

        VectorGlobalConfig & operator=(const VectorGlobalConfig &) = default;

        uint32_t getVLEN() const { return vlen_; }

        uint32_t getSEW() const { return sew_; }

        float getLMUL() const { return lmul_; }

        uint32_t getVLMAX() const { return vlmax_; }

        uint32_t getVL() const { return vl_; }

        bool getVTA() const { return vta_; }

        bool getVMA() const { return vma_; }

      private:
        uint32_t vlen_;   // vector register width in bits
        uint32_t sew_;    // element width in bits
        float lmul_;      // number of registers per group
        uint32_t vlmax_;  // max vector length
        uint32_t vl_ = 0; // vector length
        bool vta_;        // tail mode, false = undisturbed, true = agnostic
        bool vma_;        // mask mode, false = undisturbed, true = agnostic
    };

    /*!
     * \class VectorInstState
     * \brief Contains vector uop specific state
     */
    class VectorInstState
    {
      public:
        using PtrType = sparta::SpartaSharedPointer<VectorInstState>;

        VectorInstState(const uint32_t start_elem_num,
                        const boost::dynamic_bitset<> & active_elem_mask, uint32_t dlen_idx) :
            starting_elem_num_(start_elem_num),
            active_elem_mask_(active_elem_mask),
            dlen_idx_(dlen_idx)
        {
        }

        const boost::dynamic_bitset<> & getActiveElements() const { return active_elem_mask_; }

        uint32_t getNumElemsThisUop() const { return active_elem_mask_.count(); }

        uint32_t getStartingElemNum() const { return starting_elem_num_; }

        uint32_t getDlenIdx() const { return dlen_idx_; }

      private:
        const uint32_t starting_elem_num_;               // Starting element number for this uop
        const boost::dynamic_bitset<> active_elem_mask_; // 1=active, 0=masked/tail
        const uint32_t dlen_idx_;                        // DLEN index for when VLEN > DLEN
    };

    using VectorInstStatePtr = VectorInstState::PtrType;

    inline std::ostream & operator<<(std::ostream & os,
                                     const vector_types::VectorGlobalConfig & vec_global_config)
    {
        os << "e" << vec_global_config.getSEW();
        if (vec_global_config.getLMUL() >= 1.0)
        {
            os << " m" << int(vec_global_config.getLMUL());
        }
        else
        {
            os << " mf" << int(1.0 / vec_global_config.getLMUL());
        }
        os << (vec_global_config.getVTA() ? " ta" : "") << (vec_global_config.getVMA() ? " ma" : "")
           << " vl: " << vec_global_config.getVL() << " vlmax: " << vec_global_config.getVLMAX();
        return os;
    }

    inline std::ostream & operator<<(std::ostream & os,
                                     const vector_types::VectorGlobalConfig* vec_global_config)
    {
        if (vec_global_config)
        {
            os << *vec_global_config;
        }
        else
        {
            os << "nullptr";
        }
        return os;
    }

    inline std::ostream & operator<<(std::ostream & os,
                                     const vector_types::VectorInstState & vec_inst_state)
    {
        os << "numElem:" << vec_inst_state.getNumElemsThisUop()
           << " startElem:" << vec_inst_state.getStartingElemNum()
           << " elems:" << vec_inst_state.getActiveElements()
           << " dlen idx:" << vec_inst_state.getDlenIdx();
        return os;
    }

    inline std::ostream & operator<<(std::ostream & os,
                                     const vector_types::VectorInstState* vec_inst_state)
    {
        if (vec_inst_state)
        {
            os << *vec_inst_state;
        }
        else
        {
            os << "nullptr";
        }
        return os;
    }
} // namespace olympia::vector_types
