cmake_minimum_required(VERSION 3.27)
project(assembler C)

set(CMAKE_C_STANDARD 11)

add_executable(assembler assembler.c
        header.h
        preprocessor.c
        list.c
        firstPass.c
        common.c
        structs.h
        secondPass.c
        typedef.h
        sentence.c
        static_data.c
        tests/test.c
)
