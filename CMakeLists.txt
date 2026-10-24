cmake_minimum_required(VERSION 3.10)
project(PP_LABS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package(OpenMP REQUIRED)
find_package(MPI QUIET COMPONENTS CXX)

add_executable(lab2_omp main_omp.cpp)
target_link_libraries(lab2_omp PRIVATE OpenMP::OpenMP_CXX)
target_compile_options(lab2_omp PRIVATE ${OpenMP_CXX_FLAGS})
target_link_options(lab2_omp PRIVATE ${OpenMP_CXX_FLAGS})
target_link_libraries(lab2_omp PRIVATE gomp)

if(MPI_CXX_FOUND)
    add_executable(lab2_mpi main_mpi.cpp)
    target_link_libraries(lab2_mpi PRIVATE MPI::MPI_CXX)
else()
    message(WARNING "MPI not found: lab2_mpi target is skipped. Install MPICH/OpenMPI to build the MPI program.")
endif()
