cmake_minimum_required(VERSION 3.20)
project(PP_LABS CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP REQUIRED)

# Настройка MPICH2
set(MPICH2_DIR "C:/Program Files/MPICH2")
include_directories("${MPICH2_DIR}/include")
link_directories("${MPICH2_DIR}/lib")

# Сборка OpenMP
add_executable(lab1_omp main_omp.cpp)

# Явно задаем флаг -fopenmp для GCC
target_compile_options(lab1_omp PRIVATE ${OpenMP_CXX_FLAGS})
target_link_options(lab1_omp PRIVATE ${OpenMP_CXX_FLAGS})
target_link_libraries(lab1_omp PRIVATE gomp)

# Сборка MPI
add_executable(lab1_mpi main_mpi.cpp)
target_link_libraries(lab1_mpi PRIVATE mpi)