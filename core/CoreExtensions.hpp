// <CoreExtension.h> -*- C++ -*-

#pragma once

#include <vector>
#include <string>
#include <memory>

#include "vector/VectorUtils.hpp"

#include "sparta/simulation/TreeNodeExtensions.hpp"
#include "sparta/simulation/ParameterSet.hpp"

namespace olympia
{

    //
    // \class CoreExtensions
    // \brief Common extensions for a specific core
    //
    // Similar to Parameters, Extensions allow the modeler to provide
    // common "preferences" to any node (and it's children).  For
    // example, the topology of the execution units: the number of
    // ALUs.  Both Dispatch and Execute (as well as testers) need to
    // know this information.
    //
    //
    class CoreExtensions : public sparta::ExtensionsParamsOnly
    {
      public:
        static constexpr char name[] = "core_extensions";

        using ExecutionTopology = std::vector<std::vector<std::string>>;
        using ExecutionTopologyParam = sparta::Parameter<ExecutionTopology>;

        using PipeTopology = std::vector<std::vector<std::string>>;
        using PipeTopologyParam = sparta::Parameter<PipeTopology>;

        using IssueQueueTopology = std::vector<std::vector<std::string>>;
        using IssueQueueTopologyParam = sparta::Parameter<IssueQueueTopology>;

        CoreExtensions() : sparta::ExtensionsParamsOnly() {}

        virtual ~CoreExtensions() {}

        void postCreate() override
        {
            sparta::ParameterSet* ps = getParameters();

            //
            // Example of an execution topology:
            //  [["alu", "1"], ["fpu", "1"], ["br",  "1"]]
            //
            //  LSU is its own entity at this time
            //
            execution_topology_.reset(
                new ExecutionTopologyParam("execution_topology", ExecutionTopology(),
                                           "Topology Post Dispatch -- the execution pipes. "
                                           "Expect: [[\"<unit_name>\", \"<count>\"]] ",
                                           ps));
            pipelines_.reset(new PipeTopologyParam("pipelines", PipeTopology(),
                                                   "Topology Mapping"
                                                   "Mapping of Pipe Targets to execution unit",
                                                   ps));
            vlen_.reset(new sparta::Parameter<uint32_t>(
                "vlen", 0, "Width of vector registers in bits (128, 256, 512, or 1024)", ps));
            vlen_->addDependentValidationCallback(&CoreExtensions::validateVlen_,
                                                  "VLEN constraint");

            dlen_.reset(new sparta::Parameter<uint32_t>(
                "dlen", 0, "Width of vector execution units in bits (must be <= VLEN)", ps));
            dlen_->addDependentValidationCallback(&CoreExtensions::validateDlen_,
                                                  "DLEN constraint");
	    issue_queue_to_pipe_map_.reset(
                new IssueQueueTopologyParam("issue_queue_to_pipe_map", IssueQueueTopology(),
                                            "Issue Queue Topology"
                                            "Defines Issue Queue to Execution Unit Mapping",
                                            ps));
            exe_pipe_rename_.reset(new IssueQueueTopologyParam("exe_pipe_rename",
                                                              IssueQueueTopology(),
                                                              "Rename for ExecutionPipes"
                                                              "Defines alias for ExecutionPipe",
                                                              ps));
            issue_queue_rename_.reset(new IssueQueueTopologyParam("issue_queue_rename",
                                                                 IssueQueueTopology(),
                                                                 "Rename for IssueQueues"
                                                                 "Defines alias for IssueQueue",
                                                                 ps));
        }

      private:
        std::unique_ptr<ExecutionTopologyParam> execution_topology_;
        std::unique_ptr<PipeTopologyParam> pipelines_;
        std::unique_ptr<sparta::Parameter<uint32_t>> vlen_;
        std::unique_ptr<sparta::Parameter<uint32_t>> dlen_;
        std::unique_ptr<IssueQueueTopologyParam> issue_queue_to_pipe_map_;
        std::unique_ptr<PipeTopologyParam> exe_pipe_rename_;
        std::unique_ptr<PipeTopologyParam> issue_queue_rename_;
 
	static bool validateVlen_(uint32_t & vlen_val, const sparta::TreeNode*)
        {
            // VLEN can be zero if vector is disabled
            if (vlen_val == 0)
            {
                return true;
            }
            return vector_utils::isVlenValid(vlen_val);
        };

        static bool validateDlen_(uint32_t & dlen_val, const sparta::TreeNode*)
        {
            // DLEN can be zero if vector is disabled
            if (dlen_val == 0)
            {
                return true;
            }
            return vector_utils::isDlenValid(dlen_val);
        };
    };
}
