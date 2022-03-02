cmake_minimum_required(VERSION 3.19)
project(updatedClientServer)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(updatedClientServer
        LocalContext.h
        main.cpp
        RPCImpl.cpp
        RPCImpl.h
        RPCServer.cpp
        RPCServer.h)
