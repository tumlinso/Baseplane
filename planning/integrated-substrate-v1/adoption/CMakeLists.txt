cmake_minimum_required(VERSION 3.20)
project(baseplane_adoption LANGUAGES CXX)
enable_testing()
get_filename_component(BP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(BP_ADOPTION_INSTALLED_PREFIX "" CACHE PATH "Optional installed Baseplane package check")
if(BP_ADOPTION_INSTALLED_PREFIX)
    find_package(Baseplane CONFIG REQUIRED PATHS "${BP_ADOPTION_INSTALLED_PREFIX}" NO_DEFAULT_PATH)
else()
    set(BASEPLANE_ENABLE_CUDA OFF CACHE BOOL "Host adoption check" FORCE)
    add_subdirectory("${BP_ROOT}" baseplane EXCLUDE_FROM_ALL)
endif()
add_executable(bp_adoption_seq_consumer seq_consumer.cpp)
target_link_libraries(bp_adoption_seq_consumer PRIVATE Baseplane::seq)
add_test(NAME bp_adoption_seq_consumer COMMAND bp_adoption_seq_consumer)
# Reuse a finished native composition with its selected numerical components.
# The dependency adapter gets a real compiled consumer, rather than mock targets.
set(CE_MOON_SOURCE_DIR "" CACHE PATH "Inspected Cellerator experimental providers")
if(CE_MOON_SOURCE_DIR)
    add_subdirectory("${CE_MOON_SOURCE_DIR}" ce-provider EXCLUDE_FROM_ALL)
    baseplane_add_representation_target(bp_adoption_representation
        COMPONENTS Cellerator::moonshot_effects)
    add_executable(bp_adoption_effect_consumer
        "${BP_ROOT}/experiments/moonshot/compositions/sequence_effects/tests/smoke.cpp")
    target_include_directories(bp_adoption_effect_consumer PRIVATE
        "${BP_ROOT}/experiments/moonshot/include"
        "${BP_ROOT}/experiments/moonshot/families")
    target_link_libraries(bp_adoption_effect_consumer PRIVATE bp_adoption_representation)
    add_test(NAME bp_adoption_effect_consumer COMMAND bp_adoption_effect_consumer)
endif()
