cmake_minimum_required(VERSION 3.20)
project(baseplane_missing_installed_provider LANGUAGES CXX)
find_package(Baseplane CONFIG REQUIRED)
include("${SUBSTRATE_MODULE}")
_baseplane_substrate_check_graph(Baseplane::seq "")
# The real standalone package has no CE capability. No mock target is created.
_baseplane_substrate_require_provider(Cellerator::moonshot_effects)
