cmake_minimum_required(VERSION 3.10)
project(ompadvanced)
#enable_language()

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake-modules)


################################################################
### Set options for compilation                              ###
################################################################
option(BUILD_SHARED_LIBS  "Sets shared linking on/off"                          ON)
option(BUILD_EXAMPLES     "Build examples"                                      OFF)
option(ENABLE_TESTS       "Enable testing"                                      OFF )
option(USE_OpenMP         "Use OpenMP"                                          ON)
option(DOWNLOAD_HDF5      "Download and install HDF5 if not found"              OFF)
option(DOWNLOAD_SPDLOG    "Download and install spdlog if not found"            OFF)
option(DOWNLOAD_EIGEN3    "Download and install Eigen3 if not found"            ON)
option(DOWNLOAD_ALL       "Download and install all dependencies if not found"  ON )

#################################################################
### Set default policies if CMake is new enough               ###
#################################################################
if (CMAKE_VERSION VERSION_LESS 3.12)
    message(STATUS "Not setting policies")
else()
    cmake_policy(SET CMP0074 NEW)
    cmake_policy(SET CMP0075 NEW)
    # Use cmake_policy CMP0067 to let try_compile honor current CMAKE_CXX_STANDARD setting.
    # See here https://stackoverflow.com/questions/47213356/cmake-using-corrext-c-standard-when-checking-for-header-files
    cmake_policy(SET CMP0067 NEW)
endif()


if(USE_MKL AND NOT DEFINED ENV{MKLROOT})
    set(ENV{MKLROOT}  "/opt/intel/mkl")
    message(STATUS "MKLROOT is not defined. Setting default: $ENV{MKLROOT}")
endif()


################################################################
### Get git version number                                   ###
### Generates a header gitversion/gitversion.h               ###
### Include it using #include <gitversion.h>                 ###
### Gives a namespace GIT:: with several git version numbers.###
################################################################
include(cmake-modules/gitversion.cmake)



##################################################################
### Get operating system properties into a variable            ###
##################################################################
cmake_host_system_information(RESULT _host_name QUERY HOSTNAME)

set(GET_OS_INFO_CMD lsb_release -a)
if(${CMAKE_HOST_APPLE})
    set(GET_OS_INFO_CMD "sw_vers")
endif()
execute_process(COMMAND ${GET_OS_INFO_CMD}
        OUTPUT_VARIABLE OS_PROPERTIES
        OUTPUT_STRIP_TRAILING_WHITESPACE)
message("========= DETECTED OS =========")
message("Hostname: " ${_host_name})
message("${OS_PROPERTIES}")
message("===============================")




###################
### Setup flags ###
###################
message("Compiling with: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

if(NOT MARCH)
    set(MARCH "native")
endif()

if(MARCH)
    if (${MARCH} STREQUAL "zen")
        string(TOUPPER ZEN OPENBLAS_MARCH)
        string(TOLOWER znver1 CXX_MARCH)
    elseif (${MARCH} STREQUAL "native")
        set(OPENBLAS_MARCH "haswell")
        string(TOLOWER ${MARCH} CXX_MARCH)
    else()
        string(TOUPPER ${MARCH} OPENBLAS_MARCH)
        string(TOLOWER ${MARCH} CXX_MARCH)
    endif()
endif()

message("Using microarchitechture: ${CXX_MARCH}")
set(COMMON_OPTIONS -g -Wall -march=${CXX_MARCH} -mtune=${CXX_MARCH})
set(DEBUG_OPTIONS            -Wpedantic -Wextra -O0 -g3 -fstack-protector -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2)
set(RELWITHDEBINFO_OPTIONS   -Wpedantic -Wextra -O1 -g3 -fstack-protector -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2)
set(RELEASE_OPTIONS          -DNDEBUG -O3 )
set(PROFILE_OPTIONS          -DNDEBUG -O3 -lprofiler -g  -ftime-report)



#################################
### Microarch dependent flags ###
#################################
# Check supported options here http://man7.org/linux/man-pages/man1/gcc.1.html
if(${MARCH} STREQUAL "haswell" OR ${MARCH} STREQUAL "skylake" OR ${MARCH} STREQUAL "zen" )
    #list(APPEND RELEASE_OPTIONS -mmmx -msse -msse2 -msse3 -mssse3 -msse4.2 -msse4.1 -mpopcnt -maes -mpclmul -mavx -mavx2 -mmovbe -mrdrnd -mf16c -mfsgsbase -mfma  -mbmi -mbmi2 -fstack-protector-strong -Wformat -Wformat-security)
else()
    # Set default to whatever sandybridge can manage
    #list(APPEND RELEASE_OPTIONS  -mmmx -msse -msse2 -msse3 -mssse3 -msse4.2 -msse4.1  -mpopcnt  -maes -mpclmul -mavx -fstack-protector-strong -Wformat -Wformat-security)
endif()


##########################
### OS-dependent flags ###
##########################
if(${CMAKE_HOST_APPLE})
    list(APPEND COMMON_OPTIONS -mmacosx-version-min=10.13)
else()
#    list(APPEND COMMON_OPTIONS -mavx ) #-mfma makes things slower?
endif()


################################
### Compiler-dependent flags ###
################################
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" )
    list(APPEND COMMON_OPTIONS -Wno-implicit-fallthrough -Wno-deprecated-declarations -Wno-ignored-attributes
            -Wno-int-in-bool-context -Wno-maybe-uninitialized -Wno-enum-compare -Wno-unused-local-typedefs)
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    list(APPEND COMMON_OPTIONS ${GCC_TOOLCHAIN})
    list(APPEND COMMON_OPTIONS -stdlib=libstdc++
            -Wno-deprecated-declarations -Wno-ignored-attributes -Wno-invalid-partial-specialization -Wno-missing-braces -Wno-unused-parameter
            -Wno-overloaded-virtual -Wno-uninitialized -Wno-unused-local-typedef -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-format-pedantic)
endif()


#######################################
### Compiler-dependent linker flags ###
#######################################
set(COMPILER_LFLAGS)
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" )
    list(APPEND COMPILER_LFLAGS -lstdc++fs)     ### For experimental::filesystem
    list(APPEND COMPILER_LFLAGS -flto)          ### Link time optimization flag
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    list(APPEND COMPILER_LFLAGS -stdlib=libstdc++)
    list(APPEND COMPILER_LFLAGS -lstdc++fs)        ### For experimental::filesystem
endif()




################################
### print current CXX flags  ###
################################
message(STATUS "COMMON CXX flags:  ${COMMON_OPTIONS} ${CMAKE_CXX_FLAGS}")
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "RELEASE CXX flags: ${RELEASE_OPTIONS}")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message(STATUS "RELWITHDEBINFO CXX flags: ${RELWITHDEBINFO_OPTIONS}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "DEBUG CXX flags: ${DEBUG_OPTIONS}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Profile")
    message(STATUS "PROFILE CXX flags: ${PROFILE_OPTIONS}")
endif()





###########################################
###  Add all source files recursively   ###
###########################################
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC src)

################################################
###  Force cmake to find .a library suffixes ###
################################################
if(BUILD_SHARED_LIBS)
    set(CUSTOM_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CUSTOM_SUFFIX} ${CMAKE_FIND_LIBRARY_SUFFIXES})
else()
    set(CUSTOM_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CUSTOM_SUFFIX} ${CMAKE_FIND_LIBRARY_SUFFIXES})
    target_link_libraries  (${PROJECT_NAME} PRIVATE -static)                                             ### Static linkage
endif()





###############################
###  Require c++17 support  ###
###############################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
set_target_properties(${PROJECT_NAME}
        PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)



######################################
###  Check  experimental headers  ###
######################################
include(cmake-modules/CheckExperimentalHeaders.cmake)
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(TEST_REQUIRED_FLAGS     "-std=c++17 -lstdc++fs" )
    set(TEST_REQUIRED_LIBRARIES "")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(TEST_REQUIRED_FLAGS     "-stdlib=libstdc++ -lstdc++fs -std=c++17 ${GCC_TOOLCHAIN}" )
    set(TEST_REQUIRED_LIBRARIES "-stdlib=libstdc++ -lstdc++fs -std=c++17")
endif()
CheckExperimentalHeaders("${TEST_REQUIRED_FLAGS}" "${TEST_REQUIRED_LIBRARIES}")


##################################################
###  Adapt pthread linking for static/dynamic  ###
##################################################
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG FALSE)
find_package(Threads REQUIRED)
if(NOT BUILD_SHARED_LIBS)
    set_target_properties(Threads::Threads PROPERTIES INTERFACE_LINK_LIBRARIES "-Wl,--whole-archive ${CMAKE_THREAD_LIBS_INIT} -Wl,--no-whole-archive")
endif()




##############################################
###  Optional OpenMP support               ###
###  Note that Clang has some              ###
###  trouble with static openmp            ###
###  Also, installing libiomp5 might help  ###
###  for shared linking.                   ###
##############################################
include(cmake-modules/CheckOMPCompiles.cmake)
if(USE_OpenMP)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenMP_LIBRARIES})
        target_compile_options(${PROJECT_NAME} PRIVATE ${OpenMP_FLAGS})
else()
    list(APPEND COMMON_OPTIONS -Wno-unknown-pragmas)
endif()


###########################################
###  Apply RELEASE/DEBUG compile flags  ###
###########################################
target_compile_options (${PROJECT_NAME} PRIVATE ${COMMON_OPTIONS})                                          ### common options
target_compile_options (${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>")                      ### debug build options
target_compile_options (${PROJECT_NAME} PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>")                  ### Release build options
target_compile_options (${PROJECT_NAME} PRIVATE "$<$<CONFIG:RELWITHDEBINFO>:${RELWITHDEBINFO_OPTIONS}>")    ### Release build with debug info options
target_compile_options (${PROJECT_NAME} PRIVATE "$<$<CONFIG:PROFILE>:${PROFILE_OPTIONS}>")                  ### Profiling build options


##########################################
###  Find or fetch all dependencies   ###
##########################################
set(BUILD_DIRECTORY   ${PROJECT_SOURCE_DIR}/cmake-build-libs)
set(INSTALL_DIRECTORY ${PROJECT_SOURCE_DIR}/libs)

include(cmake-modules/Fetch_Eigen3.cmake)                   # Eigen library
include(cmake-modules/Fetch_spdlog.cmake)                   # spdlog library

target_link_libraries(${PROJECT_NAME} PRIVATE Eigen3::Eigen)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE ${COMPILER_LFLAGS})

get_target_property(EIGEN_INFO          Eigen3::Eigen    INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(SPDLOG_INFO         spdlog::spdlog   INTERFACE_INCLUDE_DIRECTORIES)





message("=============================================================================")
message("=== SUMMARY OF CMAKE CONFIGURATION:  THE FOLLOWING LIBRARIES WILL BE USED ===")
message("=============================================================================")
message("    EIGEN3         INC:  ${EIGEN_INFO}")
message("-----------------------------------------------------------------------------")
message("    SPDLOG         INC:  ${SPDLOG_INFO}")
message("=============================================================================")
message("")



###################
### INFORMATION ###
###################
message("")
message("=============================================================================")
message("")
message("      To clean cmake files, run './build.sh -c'")
message("      To clean cmake files downloaded libraries, run './build.sh -l '")
message("      To launch this program,  './run.sh'")
message("")
message("=============================================================================")
message("")



#############################
## UNIT TEST
#############################
if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests/hello EXCLUDE_FROM_ALL)

    add_custom_target(TEST-all ALL
            DEPENDS TEST-hello
            DEPENDS ${PROJECT_NAME}
            )

    add_custom_command(TARGET TEST-all
            POST_BUILD
            COMMENT "Running Tests"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS Eigen3::Eigen spdlog::spdlog
            COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> --output-on-failures)

endif()





 
