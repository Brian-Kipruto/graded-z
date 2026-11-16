# ============================================================================
#  CMakeLists.txt  —  Graded-Z Shielding (in-silico Phase 1)
# ----------------------------------------------------------------------------
#  GLOB-based source discovery (convenient for a fast-moving thesis project).
#  GOTCHA: file(GLOB) does NOT auto-detect newly added .cc/.hh files. After
#  adding a new source file you MUST re-run `cmake ..` once before `make`,
#  or you'll get "undefined reference" linker errors. (CMake Build.md)
# ============================================================================
cmake_minimum_required(VERSION 3.16)
project(graded-z)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Pull in Geant4 with all UI + vis drivers.
find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

# Headers.
include_directories(${PROJECT_SOURCE_DIR}/include)

# Sources (GLOB — see gotcha above).
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

# The single executable.
add_executable(graded-z main.cc ${sources} ${headers})
target_link_libraries(graded-z ${Geant4_LIBRARIES})

# Copy macros next to the binary so it can find vis.mac / source macros at runtime.
file(GLOB macro_files ${PROJECT_SOURCE_DIR}/macros/*.mac)
foreach(macro ${macro_files})
    get_filename_component(macro_name ${macro} NAME)
    configure_file(${macro} ${PROJECT_BINARY_DIR}/${macro_name} COPYONLY)
endforeach()

# Make a data/runs dir in the build tree so default runs have somewhere to write.
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/data/runs)
