# set min version
cmake_minimum_required(VERSION 3.11)

# setting project name
project(quizGame VERSION 1.2)

# including header files via directory, change account for proper function
# include both client and server folders
#include_directories(
#        /home/st/jholbrook/milestone2.3/server
#        /home/st/jholbrook/milestone2.3/server
#)

# add the executable
add_executable(server main.cpp LocalContext.h RPCServer.h RPCImpl.h RPCImpl.cpp RPCServer.cpp)

# pthread library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries("server" PRIVATE Threads::Threads)
