cmake_minimum_required(VERSION 3.19)
project(MyShell)

set(CMAKE_CXX_STANDARD 14)

include_directories(src/CLI)
include_directories(src/Command)
include_directories(src/Exception)
include_directories(src/Job)
include_directories(src/Parser)

aux_source_directory(src/CLI CLI)
aux_source_directory(src/Command COMMAND)
aux_source_directory(src/Job JOB)
aux_source_directory(src/Parser PARSER)
aux_source_directory(src/Exception EXCEPTION)

add_executable(MyShell main.cc ${CLI} ${COMMAND} ${JOB} ${PARSER} ${EXCEPTION})
target_link_libraries(MyShell readline)