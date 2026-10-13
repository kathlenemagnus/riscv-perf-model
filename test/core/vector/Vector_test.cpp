#include "OlympiaSim.hpp"
#include "Inst.hpp"
#include "decode/Decode.hpp"
#include "ROB.hpp"
#include "vector/VectorUopGenerator.hpp"

#include "sparta/app/CommandLineSimulator.hpp"
#include "sparta/kernel/Scheduler.hpp"
#include "sparta/simulation/ClockManager.hpp"
#include "sparta/utils/SpartaSharedPointer.hpp"
#include "sparta/utils/SpartaTester.hpp"

const char USAGE[] = "Usage:\n"
                     "\n"
                     "\n";

class olympia::DecodeTester
{
  public:
    DecodeTester(olympia::Decode* decode) : decode_(decode) {}

    void test_vl(const uint32_t expected_vl)
    {
        EXPECT_FALSE(decode_->vec_global_config_.empty());
        EXPECT_EQUAL(decode_->vec_global_config_.front().second.getVL(), expected_vl);
    }

    void test_sew(const uint32_t expected_sew)
    {
        EXPECT_FALSE(decode_->vec_global_config_.empty());
        EXPECT_EQUAL(decode_->vec_global_config_.front().second.getSEW(), expected_sew);
    }

    void test_lmul(const uint32_t expected_lmul)
    {
        EXPECT_FALSE(decode_->vec_global_config_.empty());
        EXPECT_EQUAL(decode_->vec_global_config_.front().second.getLMUL(), expected_lmul);
    }

    void test_vlmax(const uint32_t expected_vlmax)
    {
        EXPECT_FALSE(decode_->vec_global_config_.empty());
        EXPECT_EQUAL(decode_->vec_global_config_.front().second.getVLMAX(), expected_vlmax);
    }

    void test_vta(const bool expected_vta)
    {
        EXPECT_FALSE(decode_->vec_global_config_.empty());
        EXPECT_EQUAL(decode_->vec_global_config_.front().second.getVTA(), expected_vta);
    }

  private:
    olympia::Decode* decode_;
};

class olympia::ROBTester
{
  public:
    ROBTester(olympia::ROB* rob) : rob_(rob) {}

    void test_num_insts_retired(const uint64_t expected_num_insts_retired)
    {
        EXPECT_EQUAL(rob_->num_retired_, expected_num_insts_retired);
    }

    void test_num_uops_retired(const uint64_t expected_num_uops_retired)
    {
        EXPECT_EQUAL(rob_->num_uops_retired_, expected_num_uops_retired);
    }

  private:
    olympia::ROB* rob_;
};

class olympia::VectorUopGeneratorTester
{
  public:
    VectorUopGeneratorTester(olympia::VectorUopGenerator* vuop) : vuop_{vuop} {}

    void test_num_vuops_generated(const uint64_t expected_num_vuops_generated)
    {
        EXPECT_EQUAL(vuop_->vuops_generated_, expected_num_vuops_generated);
    }

  private:
    VectorUopGenerator* vuop_;
};

void runTests(int argc, char** argv)
{
    sparta::app::DefaultValues DEFAULTS;
    DEFAULTS.auto_summary_default = "off";
    DEFAULTS.arch_search_dirs = {"arches"};
    std::string input_file;
    uint32_t expected_num_uops;

    sparta::app::CommandLineSimulator cls(USAGE, DEFAULTS);
    auto & app_opts = cls.getApplicationOptions();
    app_opts.add_options()
        ("input-file",
         sparta::app::named_value<std::string>("INPUT_FILE", &input_file)->default_value(""),
         "Provide a JSON instruction stream",
         "Provide a JSON file with instructions to run through Execute")
        ("expected-num-uops",
         sparta::app::named_value<uint32_t>("EXPECTED_NUM_UOPS", &expected_num_uops)->default_value(0),
         "");

    int err_code = 0;
    if (!cls.parse(argc, argv, err_code))
    {
        sparta_assert(false, "Command line parsing failed");
    }

    sparta::Scheduler scheduler;
    const uint64_t num_cores = 1;
    const uint64_t ilimit = 0;
    const bool show_factories = false;
    OlympiaSim sim(scheduler,
                   num_cores,
                   input_file,
                   ilimit,
                   show_factories);
    sparta::RootTreeNode *root_node = sim.getRoot();
    cls.populateSimulation(&sim);

    auto* my_decode = root_node->getChild("cpu.core0.decode")->getResourceAs<olympia::Decode*>();
    olympia::DecodeTester decode_tester{my_decode};

    auto* my_rob = root_node->getChild("cpu.core0.rob")->getResourceAs<olympia::ROB*>();
    olympia::ROBTester retire_tester{my_rob};

    auto* my_vuop_generator = root_node->getChild("cpu.core0.decode.vec_uop_gen")
                                  ->getResourceAs<olympia::VectorUopGenerator*>();
    olympia::VectorUopGeneratorTester vuop_tester{my_vuop_generator};

    if (input_file.find("vsetivli_vaddvv_e8m4.json") != std::string::npos)
    {
        // Test Decode (defaults)
        decode_tester.test_lmul(1);
        decode_tester.test_vl(16);
        decode_tester.test_vta(false);
        decode_tester.test_sew(8);
        decode_tester.test_vlmax(16);

        cls.runSimulator(&sim);

        // Test after vsetivli
        decode_tester.test_lmul(4);
        decode_tester.test_vl(64);
        decode_tester.test_vta(false);
        decode_tester.test_sew(8);
        decode_tester.test_vlmax(64);

        // Test Vector Uop Generation
        vuop_tester.test_num_vuops_generated(expected_num_uops);

        // Test Retire
        retire_tester.test_num_insts_retired(2);
        // vset + 4 vadd.vv uops
        retire_tester.test_num_uops_retired(5);
    }
    else if (input_file.find("vsetvli_vaddvv_e32m1ta.json") != std::string::npos)
    {
        cls.runSimulator(&sim);

        // Test Decode
        decode_tester.test_lmul(1);
        decode_tester.test_vl(4);
        decode_tester.test_vta(true);
        decode_tester.test_sew(32);
        decode_tester.test_vlmax(4);

        // Test Vector Uop Generation
        vuop_tester.test_num_vuops_generated(expected_num_uops);

        // Test Retire
        retire_tester.test_num_insts_retired(2);
        // vset + 1 vadd.vv uop
        retire_tester.test_num_uops_retired(2);
    }
    else if (input_file.find("vsetvl_vaddvv_e64m1ta.json") != std::string::npos)
    {
        cls.runSimulator(&sim);

        // Test Decode
        decode_tester.test_lmul(1);
        decode_tester.test_vl(2);
        decode_tester.test_vta(true);
        decode_tester.test_sew(64);
        decode_tester.test_vlmax(2);

        // Test Vector Uop Generation
        vuop_tester.test_num_vuops_generated(expected_num_uops);

        // Test Retire
        retire_tester.test_num_insts_retired(2);
        // vset + 1 vadd.vv uop
        retire_tester.test_num_uops_retired(2);
    }
    else if (input_file.find("vsetivli_vaddvv_tail_e8m8ta.json") != std::string::npos)
    {
        cls.runSimulator(&sim);

        // Test Decode
        decode_tester.test_lmul(8);
        decode_tester.test_vl(111);
        decode_tester.test_vta(false);
        decode_tester.test_sew(8);
        decode_tester.test_vlmax(128);

        // Test Vector Uop Generation
        vuop_tester.test_num_vuops_generated(expected_num_uops);
    }
    else if (input_file.find("multiple_vset.json") != std::string::npos)
    {
        cls.runSimulator(&sim);

        // Test Decode (last vset)
        decode_tester.test_lmul(8);
        decode_tester.test_vl(32);
        decode_tester.test_vta(false);
        decode_tester.test_sew(32);
        decode_tester.test_vlmax(32);

        // Test Vector Uop Generation
        vuop_tester.test_num_vuops_generated(expected_num_uops);

        // Test Retire
        retire_tester.test_num_insts_retired(8);
        // vset + 1 vadd.vv + vset + 2 vadd.vv + vset + 4 vadd.vv uop + vset + 8 vadd.vv
        retire_tester.test_num_uops_retired(19);
    }
    else if (input_file.find("vrgather.json") != std::string::npos)
    {
        // Unsupported vector instructions are expected to make the simulator to throw
        bool sparta_exception_fired = false;
        try
        {
            cls.runSimulator(&sim);
        }
        catch (...)
        {
            sparta_exception_fired = true;
        }
        EXPECT_TRUE(sparta_exception_fired);
    }
    else
    {
        cls.runSimulator(&sim);

        // Test Vector Uop Generation
        vuop_tester.test_num_vuops_generated(expected_num_uops);
    }
}

int main(int argc, char** argv)
{
    TEST_INIT

    runTests(argc, argv);

    REPORT_ERROR;
    return (int)ERROR_CODE;
}
