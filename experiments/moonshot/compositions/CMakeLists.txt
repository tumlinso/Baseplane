cmake_minimum_required(VERSION 3.18)

# The standalone entry imports the parent substrate. If a parent later includes
# this directory during that import, defer discovery to the original entry.
get_property(bp_moon_compositions_entered GLOBAL PROPERTY BP_MOON_COMPOSITIONS_ENTERED)
if(bp_moon_compositions_entered)
  return()
endif()
set_property(GLOBAL PROPERTY BP_MOON_COMPOSITIONS_ENTERED TRUE)

if(NOT TARGET Baseplane::moonshot)
  project(baseplane_moonshot_compositions LANGUAGES CXX)
  include(CTest)
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/.." "${CMAKE_CURRENT_BINARY_DIR}/substrate")
endif()
if(NOT TARGET Baseplane::moonshot)
  message(FATAL_ERROR "Composition entry requires the Baseplane::moonshot substrate")
endif()

# A composition owns its executable, provider links, tests and receipts locally.
file(GLOB bp_moon_composition_builds CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/*/CMakeLists.txt")
foreach(bp_moon_composition_build IN LISTS bp_moon_composition_builds)
  get_filename_component(bp_moon_composition_dir "${bp_moon_composition_build}" DIRECTORY)
  get_filename_component(bp_moon_composition_name "${bp_moon_composition_dir}" NAME)
  add_subdirectory("${bp_moon_composition_dir}" "${bp_moon_composition_name}")
endforeach()
