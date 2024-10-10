# ************************************* UNIT TESTS *********************************************** #

# Unit tests are currently using gtest library
set(
        UNIT_TEST_SOURCE_FILES

        unit_tests.cpp

        # NOTE THAT: Alphabetic order of dirs must be preserved.

        src_test/algorithms/depth_first_search_test.cpp
        src_test/algorithms/top_sort_test.cpp

        src_test/common/operators_test.cpp
        src_test/common/nt_operators_test.cpp

        src_test/parser/bench_parser_test.cpp

        src_test/simplification/utils/two_coloring.cpp
        src_test/simplification/utils/three_coloring.cpp

        src_test/simplification/redundant_gates_cleaner.cpp
        src_test/simplification/reduce_not_composition.cpp
        src_test/simplification/duplicate_operands_cleaner.cpp
        src_test/simplification/constant_gate_reducer.cpp
        src_test/simplification/duplicate_gates_cleaner.cpp

        src_test/structures/assignment/vector_assignment_test.cpp
        src_test/structures/circuit/dag_test.cpp

        src_test/utility/encoder_test.cpp
)

add_executable(UnitTests ${UNIT_TEST_SOURCE_FILES})
target_link_libraries(UnitTests gtest gtest_main)
