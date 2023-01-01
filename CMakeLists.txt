cmake_minimum_required(VERSION 3.22)
project(Compiler)

set(CMAKE_CXX_STANDARD 11)
include_directories(./include)
add_executable(Compiler main.cpp Lexer.cpp Parser.cpp ErrorProcesser.cpp SymbolTable.cpp IRCodeGenerator.cpp
        MipsGenerator.cpp Optimization.cpp BigInteger.cpp BasicBlock.cpp Register.cpp)
