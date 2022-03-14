# set min version
cmake_minimum_required(VERSION 3.11)

# setting project name
project(milestone2 VERSION 1.2)

# add directories that contain the other cmakelists.txt files
add_subdirectory(server)
add_subdirectory(client)