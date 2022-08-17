cmake_minimum_required(VERSION 3.0.0)
project(HashColon VERSION 1.0.0)

# Build options
set(CMAKE_VERBOSE_MAKEFILE ON)
set(HASHCOLON_TEST_CODE ON)

# ==== C/C++ standard ====
# C11, C++17 required 
set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED True)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

# ==== External libraries ====
find_package(PkgConfig REQUIRED)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)  # Eigen3
find_package(GDAL 2.2 REQUIRED)              # GDAL
find_package(OpenMP)                            # OpenMP
find_package(Boost 1.65 REQUIRED             # Boost
COMPONENTS
    filesystem
    system
)
pkg_check_modules(flann REQUIRED flann)

# Project source files
add_library(HashColon 
# HashColon
    HashColon/src/CAGD.cpp
    HashColon/src/Clustering.cpp
    HashColon/src/GeoValues.cpp
    HashColon/src/Helper.cpp
    HashColon/src/Log.cpp
    HashColon/src/Real.cpp
    HashColon/src/SingletonCLI.cpp
    HashColon/src/Statistics.cpp
    HashColon/src/Table.cpp
    HashColon/src/ThreadPool.cpp
# HashColon/Feline
    HashColon/Feline/src/AisPreprocess.cpp
    HashColon/Feline/src/FelineJsonIO.cpp
    HashColon/Feline/src/GeoData.cpp    
    # Trajectory, XTD stuffs are disabled
    # HashColon/Feline/src/TrajectoryClustering.cpp
    # HashColon/Feline/src/XtdEstimation.cpp
    # HashColon/Feline/src/XtdTrajectoryClustering.cpp
)
# set libraries
target_link_libraries(HashColon
PUBLIC     
    Eigen3::Eigen
    Boost::filesystem
    Boost::system
)
# set include directories
target_include_directories(HashColon 
PUBLIC 
    .
    ./ext  
    ${Eigen3_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}  
)

#set compiler options
target_compile_options(HashColon
PUBLIC
    -fopenmp
    -pthread
    -fPIC
    -O3
    # -pg
)

# set linker options
# target_link_options(HashColon
# PUBLIC
#     -fopenmp
#     -pthread
#     -fPIC
#     -O3
#     # -pg
# )
set_target_properties(HashColon
PROPERTIES 
    LINK_FLAGS "-fopenmp -pthread -fPIC -O3" #-pg
)

# Testing project
if(HASHCOLON_TEST_CODE)
    add_executable(HashColon_Test
        test/test.cpp
    )
    target_link_libraries(HashColon_Test 
    PUBLIC 
        HashColon
    )

    # set include directories
    target_include_directories(HashColon_Test
    PUBLIC 
        .
        ./ext  
        ${Eigen3_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}  
    )

    #set compiler options
    target_compile_options(HashColon_Test
    PUBLIC
        -fopenmp
        -pthread
        -fPIC
        -O3
        # -pg
    )

    # set linker options
    # target_link_options(HashColon_Test
    # PUBLIC
    #    -fopenmp
    #    -pthread
    #    -fPIC
    #    -O3
        # -pg
    #)
    set_target_properties(HashColon_Test
        PROPERTIES 
        LINK_FLAGS
            "-fopenmp -pthread"
    )

endif()

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)


