project(Vector_test)

add_executable(Vector_test Vector_test.cpp ${SIM_BASE}/sim/OlympiaSim.cpp)
target_link_libraries(Vector_test core common_test stf mavis SPARTA::sparta)

file(CREATE_LINK ${SIM_BASE}/mavis/json ${CMAKE_CURRENT_BINARY_DIR}/mavis_isa_files SYMBOLIC)
file(CREATE_LINK ${SIM_BASE}/arches     ${CMAKE_CURRENT_BINARY_DIR}/arches          SYMBOLIC)
file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/tests ${CMAKE_CURRENT_BINARY_DIR}/tests SYMBOLIC)

# VLEN=1024, DLEN=1024
set(TEST_YAMLS "big_core.yaml")
foreach(TEST_YAML ${TEST_YAMLS})
  string(REGEX REPLACE ".yaml" "" ARCH ${TEST_YAML})

  # Vset tests
  sparta_named_test(Vector_test_${ARCH}_vsetivli      Vector_test --arch ${TEST_YAML} --input-file tests/vsetivli_vaddvv_e8m4.json   --expected-num-uops 4)
  sparta_named_test(Vector_test_${ARCH}_vsetvli       Vector_test --arch ${TEST_YAML} --input-file tests/vsetvli_vaddvv_e32m1ta.json --expected-num-uops 1)
  sparta_named_test(Vector_test_${ARCH}_vsetvl        Vector_test --arch ${TEST_YAML} --input-file tests/vsetvl_vaddvv_e64m1ta.json  --expected-num-uops 1)
  sparta_named_test(Vector_test_${ARCH}_multiple_vset Vector_test --arch ${TEST_YAML} --input-file tests/multiple_vset.json          --expected-num-uops 15)

  # Tail tests
  sparta_named_test(Vector_test_${ARCH}_vsetivli_tail_uops    Vector_test --arch ${TEST_YAML} -p top.cpu.core0.decode.vec_uop_gen.params.generate_tail_uops true  --input-file tests/vsetivli_vaddvv_tail_e8m8ta.json --expected-num-uops 8)
  sparta_named_test(Vector_test_${ARCH}_vsetivli_no_tail_uops Vector_test --arch ${TEST_YAML} -p top.cpu.core0.decode.vec_uop_gen.params.generate_tail_uops false --input-file tests/vsetivli_vaddvv_tail_e8m8ta.json --expected-num-uops 7)

  # Vector sequencing tests
  sparta_named_test(Vector_test_${ARCH}_elementwise    Vector_test --arch ${TEST_YAML} --input-file tests/elementwise.json    --expected-num-uops 8)
  sparta_named_test(Vector_test_${ARCH}_widening       Vector_test --arch ${TEST_YAML} --input-file tests/widening.json       --expected-num-uops 8)
  sparta_named_test(Vector_test_${ARCH}_widening_mixed Vector_test --arch ${TEST_YAML} --input-file tests/widening_mixed.json --expected-num-uops 8)
  sparta_named_test(Vector_test_${ARCH}_narrowing      Vector_test --arch ${TEST_YAML} --input-file tests/narrowing.json      --expected-num-uops 4)
  sparta_named_test(Vector_test_${ARCH}_mac            Vector_test --arch ${TEST_YAML} --input-file tests/mac.json            --expected-num-uops 4)
  sparta_named_test(Vector_test_${ARCH}_mac_widening   Vector_test --arch ${TEST_YAML} --input-file tests/mac_widening.json   --expected-num-uops 12)
  sparta_named_test(Vector_test_${ARCH}_single_dest    Vector_test --arch ${TEST_YAML} --input-file tests/single_dest.json    --expected-num-uops 4)
  sparta_named_test(Vector_test_${ARCH}_single_src     Vector_test --arch ${TEST_YAML} --input-file tests/single_src.json     --expected-num-uops 4)
  sparta_named_test(Vector_test_${ARCH}_reduction      Vector_test --arch ${TEST_YAML} --input-file tests/reduction.json      --expected-num-uops 8)
  sparta_named_test(Vector_test_${ARCH}_reduction_wide Vector_test --arch ${TEST_YAML} --input-file tests/reduction_wide.json --expected-num-uops 8)
  sparta_named_test(Vector_test_${ARCH}_int_ext        Vector_test --arch ${TEST_YAML} --input-file tests/int_ext.json        --expected-num-uops 24)
  sparta_named_test(Vector_test_${ARCH}_slide          Vector_test --arch ${TEST_YAML} --input-file tests/slide.json          --expected-num-uops 32)
  sparta_named_test(Vector_test_${ARCH}_permute        Vector_test --arch ${TEST_YAML} --input-file tests/permute.json        --expected-num-uops 73)

  # Vector instruction tests
  sparta_named_test(Vector_test_${ARCH}_vfirst Vector_test --arch ${TEST_YAML} --input-file tests/vfirst.json --expected-num-uops 0)
  sparta_named_test(Vector_test_${ARCH}_vmv    Vector_test --arch ${TEST_YAML} --input-file tests/vmv.json    --expected-num-uops 16)
endforeach()
