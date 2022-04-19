cmake_minimum_required(VERSION 3.0.0)
project(HashColon VERSION 1.0.0)

# ==== C/C++ standard ====
# C11, C++17 required 
set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED True)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

# ==== External libraries ====
# Eigen3
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
if(Eigen3_Found)
    include_directories(${Eigen3_INCLUDE_DIRS})
endif()
# Boost
find_package(Boost 1.65
COMPONENTS
    filesystem
    system
REQUIRED)
if(Boost_Found)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

# Project source files
add_library(HashColon 
# HashColon
    HashColon/src/CAGD.cpp
    HashColon/src/Clustering.cpp
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
    HashColon/Feline/src/GeoValues.cpp
    # Trajectory, XTD stuffs are disabled
    # HashColon/Feline/src/TrajectoryClustering.cpp
    # HashColon/Feline/src/XtdEstimation.cpp
    # HashColon/Feline/src/XtdTrajectoryClustering.cpp
)
# set libraries
target_link_libraries(HashColon
PUBLIC     
    Eigen3::Eigen
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)
# set include directories
target_include_directories(HashColon 
PUBLIC 
    .
    ./ext    
)

# Testing project
add_executable(HashColon_Test
    test/test.cpp
)
target_link_libraries(HashColon_Test PUBLIC HashColon)
#target_include_directories()




# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)


