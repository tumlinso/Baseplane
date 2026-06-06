cmake_minimum_required(VERSION 3.20)

project(Baseplane LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(BASEPLANE_ENABLE_CUDA "Enable CUDA sequence kernels and CUDA tests/benchmarks when a CUDA toolchain is available." ON)
option(BASEPLANE_ENABLE_CUDA_LINEINFO "Emit CUDA line mapping for Nsight profiling." OFF)
option(BASEPLANE_ENABLE_HIGHWAY "Enable Google Highway SIMD sequence kernels when a source tree or CMake package is available." OFF)
set(BASEPLANE_HIGHWAY_SOURCE_DIR "" CACHE PATH "Optional Google Highway source tree for SIMD sequence kernels.")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(BASEPLANE_HAS_CUDA 0)
if(BASEPLANE_ENABLE_CUDA)
    if(NOT DEFINED CMAKE_CUDA_COMPILER)
        if(DEFINED ENV{CUDACXX} AND NOT "$ENV{CUDACXX}" STREQUAL "")
            set(CMAKE_CUDA_COMPILER "$ENV{CUDACXX}" CACHE FILEPATH "CUDA compiler" FORCE)
        elseif(EXISTS "/opt/nvidia/hpc_sdk/Linux_x86_64/26.1/cuda/12.9/bin/nvcc")
            set(CMAKE_CUDA_COMPILER
                "/opt/nvidia/hpc_sdk/Linux_x86_64/26.1/cuda/12.9/bin/nvcc"
                CACHE FILEPATH "CUDA compiler" FORCE)
            message(STATUS "Defaulting CUDA compiler to the local HPC SDK nvcc")
        else()
            find_program(BASEPLANE_NVCC nvcc)
            if(BASEPLANE_NVCC)
                set(CMAKE_CUDA_COMPILER "${BASEPLANE_NVCC}" CACHE FILEPATH "CUDA compiler" FORCE)
            endif()
        endif()
    endif()
    if(NOT DEFINED CMAKE_CUDA_HOST_COMPILER)
        if(DEFINED ENV{CUDAHOSTCXX} AND NOT "$ENV{CUDAHOSTCXX}" STREQUAL "")
            set(CMAKE_CUDA_HOST_COMPILER "$ENV{CUDAHOSTCXX}" CACHE FILEPATH "CUDA host compiler" FORCE)
        elseif(EXISTS "/usr/bin/g++-12")
            set(CMAKE_CUDA_HOST_COMPILER "/usr/bin/g++-12" CACHE FILEPATH "CUDA host compiler" FORCE)
            message(STATUS "Defaulting CUDA host compiler to /usr/bin/g++-12 for nvcc compatibility")
        endif()
    endif()
    if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
        set(CMAKE_CUDA_ARCHITECTURES 70 CACHE STRING "CUDA architectures; default is native Volta sm_70.")
    endif()
    if(DEFINED CMAKE_CUDA_COMPILER AND EXISTS "${CMAKE_CUDA_COMPILER}")
        enable_language(CUDA)
        find_package(CUDAToolkit REQUIRED)
        set(BASEPLANE_HAS_CUDA 1)
    else()
        message(STATUS "Baseplane CUDA disabled because no CUDA compiler was found")
    endif()
endif()

set(BASEPLANE_SEQ_SOURCES
    src/seq/dna2_scalar.cpp
)
if(BASEPLANE_HAS_CUDA)
    list(APPEND BASEPLANE_SEQ_SOURCES src/seq/dna2.cu)
endif()

set(BASEPLANE_HAS_HIGHWAY 0)
if(BASEPLANE_ENABLE_HIGHWAY)
    if(BASEPLANE_HIGHWAY_SOURCE_DIR AND EXISTS "${BASEPLANE_HIGHWAY_SOURCE_DIR}/CMakeLists.txt")
        set(HWY_ENABLE_CONTRIB OFF CACHE BOOL "Include Highway contrib/" FORCE)
        set(HWY_ENABLE_EXAMPLES OFF CACHE BOOL "Build Highway examples" FORCE)
        set(HWY_ENABLE_INSTALL ON CACHE BOOL "Install Highway" FORCE)
        set(HWY_ENABLE_TESTS OFF CACHE BOOL "Enable Highway tests" FORCE)
        add_subdirectory("${BASEPLANE_HIGHWAY_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/highway" EXCLUDE_FROM_ALL)
        set(BASEPLANE_HAS_HIGHWAY 1)
    else()
        find_package(hwy CONFIG QUIET)
        if(TARGET hwy)
            set(BASEPLANE_HAS_HIGHWAY 1)
        else()
            message(STATUS "Baseplane Highway SIMD disabled; set BASEPLANE_HIGHWAY_SOURCE_DIR or install hwy to enable it")
        endif()
    endif()
endif()
if(BASEPLANE_HAS_HIGHWAY)
    list(APPEND BASEPLANE_SEQ_SOURCES src/seq/dna2_highway.cpp)
endif()

add_library(baseplane_seq STATIC ${BASEPLANE_SEQ_SOURCES})
add_library(Baseplane::seq ALIAS baseplane_seq)
target_include_directories(baseplane_seq PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(baseplane_seq PUBLIC cxx_std_17)
target_compile_definitions(baseplane_seq PUBLIC
    BASEPLANE_ENABLE_CUDA=${BASEPLANE_HAS_CUDA}
    BASEPLANE_ENABLE_HIGHWAY=${BASEPLANE_HAS_HIGHWAY}
)
if(BASEPLANE_HAS_CUDA)
    target_link_libraries(baseplane_seq PUBLIC CUDA::cudart)
endif()
if(BASEPLANE_HAS_HIGHWAY)
    target_link_libraries(baseplane_seq PRIVATE
        $<BUILD_INTERFACE:hwy>
        $<INSTALL_INTERFACE:hwy>
    )
endif()
set_target_properties(baseplane_seq PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    EXPORT_NAME seq
    POSITION_INDEPENDENT_CODE ON
)
if(BASEPLANE_HAS_CUDA)
    set_target_properties(baseplane_seq PROPERTIES CUDA_STANDARD 17 CUDA_STANDARD_REQUIRED YES)
endif()

function(baseplane_enable_perf target)
    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-O3 -DNDEBUG -march=native -mtune=native>
    )
    if(BASEPLANE_HAS_CUDA)
        set(baseplane_cuda_perf_flags
            -O3
            --use_fast_math
            --expt-relaxed-constexpr
            --expt-extended-lambda
            -Xptxas=-O3,-dlcm=ca
        )
        if(BASEPLANE_ENABLE_CUDA_LINEINFO)
            list(APPEND baseplane_cuda_perf_flags -lineinfo)
        endif()
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:${baseplane_cuda_perf_flags}>)
    endif()
    set_target_properties(${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE YES)
endfunction()

baseplane_enable_perf(baseplane_seq)

add_executable(baseplaneDna2Test tests/seq/test_dna2.cpp)
target_include_directories(baseplaneDna2Test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(baseplaneDna2Test PRIVATE Baseplane::seq)
set_target_properties(baseplaneDna2Test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
baseplane_enable_perf(baseplaneDna2Test)

add_executable(baseplaneDna2CpuBench bench/seq/bench_dna2_cpu.cpp)
target_include_directories(baseplaneDna2CpuBench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(baseplaneDna2CpuBench PRIVATE Baseplane::seq)
set_target_properties(baseplaneDna2CpuBench PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
baseplane_enable_perf(baseplaneDna2CpuBench)

if(BASEPLANE_HAS_CUDA)
    add_executable(baseplaneDna2CudaTest tests/seq/test_dna2_cuda.cu)
    target_include_directories(baseplaneDna2CudaTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(baseplaneDna2CudaTest PRIVATE Baseplane::seq CUDA::cudart)
    set_target_properties(baseplaneDna2CudaTest PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CUDA_STANDARD 17 CUDA_STANDARD_REQUIRED YES)
    baseplane_enable_perf(baseplaneDna2CudaTest)

    add_executable(baseplaneDna2Bench bench/seq/bench_dna2.cu)
    target_include_directories(baseplaneDna2Bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(baseplaneDna2Bench PRIVATE Baseplane::seq CUDA::cudart)
    set_target_properties(baseplaneDna2Bench PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CUDA_STANDARD 17 CUDA_STANDARD_REQUIRED YES)
    baseplane_enable_perf(baseplaneDna2Bench)
endif()

install(TARGETS baseplane_seq
    EXPORT BaseplaneTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT BaseplaneTargets
    NAMESPACE Baseplane::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Baseplane
)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/BaseplaneConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/BaseplaneConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Baseplane
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/BaseplaneConfigVersion.cmake
    VERSION 0.1.0
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/BaseplaneConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/BaseplaneConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Baseplane
)
