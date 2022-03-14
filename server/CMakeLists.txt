# set min version
cmake_minimum_required(VERSION 3.11)

# setting project name
project(milestone2 VERSION 1.2)

# add the executable
add_executable(server main.cpp LocalContext.h RPCServer.h RPCImpl.h RPCImpl.cpp RPCServer.cpp)

# pthread library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries("server" PRIVATE Threads::Threads)
