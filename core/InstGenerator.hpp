// <InstGenerator.hpp> -*- C++ -*-

//!
//! \file InstGenerator.hpp
//! \brief Definition of the CoreModel InstGenerator
//!

#pragma once

#include <string>
#include <memory>

#include "Inst.hpp"
#include "decode/MavisUnit.hpp"
#include "mavis/JSONUtils.hpp"
#include "sparta/utils/SpartaAssert.hpp"
#include "sparta/log/MessageSource.hpp"
#include "sparta/utils/LogUtils.hpp"

#include "stf-inc/stf_inst_reader.hpp"

namespace olympia
{
    /*
     * \class InstGenerator
     * \brief Instruction generator base class
     *
     * Base class used to fetch an instruction to send down the core
     * pipe.  Possible derivations are STF Instruction reader, JSON
     * generator, etc
     */
    class InstGenerator
    {
    public:
        InstGenerator(sparta::log::MessageSource & info_logger, MavisType * mavis_facade, const uint32_t vlen) :
            info_logger_(info_logger),
            mavis_facade_(mavis_facade),
	    vlen_(vlen) {}
        virtual ~InstGenerator() {}
        virtual InstPtr getNextInst(const sparta::Clock * clk) = 0;
        static std::unique_ptr<InstGenerator> createGenerator(sparta::log::MessageSource & info_logger,
                                                              MavisType * mavis_facade,
                                                              const std::string & filename,
                                                              const bool skip_nonuser_mode,
							      const uint32_t vlen);
        virtual bool isDone() const = 0;
        virtual void reset(const InstPtr &, const bool) = 0;

    protected:
        sparta::log::MessageSource & info_logger_;
        MavisType * mavis_facade_ = nullptr;
        uint64_t    unique_id_ = 0;
        uint64_t    program_id_ = 1;

	// Vector
	const uint32_t vlen_;
        uint32_t last_vl_ = 0;
	void initVectorGlobalConfig_(const InstPtr & inst_ptr, const uint32_t vtype_val,
                                     const uint32_t avl, const bool set_vl_to_vlmax,
                                     const bool set_vl_to_last_vl);
    };

    // Generates instructions from a JSON file
    class JSONInstGenerator : public InstGenerator
    {
    public:
        JSONInstGenerator(sparta::log::MessageSource & info_logger,
                          MavisType * mavis_facade,
                          const std::string & filename,
			  const uint32_t vlen);
        InstPtr getNextInst(const sparta::Clock * clk) override final;

        bool isDone() const override final;
        void reset(const InstPtr &, const bool) override final;


    private:
        boost::json::array  jobj_;
        uint64_t            curr_inst_index_ = 0;
        uint64_t            n_insts_ = 0;
    };

    // Generates instructions from an STF Trace file
    class TraceInstGenerator : public InstGenerator
    {
    public:
        // Creates a TraceInstGenerator with the given mavis facade
        // and filename.  The parameter skip_nonuser_mode allows the
        // trace generator to skip system instructions if present
        TraceInstGenerator(sparta::log::MessageSource & info_logger,
                           MavisType * mavis_facade,
                           const std::string & filename,
                           const bool skip_nonuser_mode,
			   const uint32_t vlen);

        InstPtr getNextInst(const sparta::Clock * clk) override final;

        bool isDone() const override final;
        void reset(const InstPtr &, const bool) override final;
    private:
        std::unique_ptr<stf::STFInstReader> reader_;

        // Always points to the *next* stf inst
        stf::STFInstReader::iterator next_it_;
    };
}
