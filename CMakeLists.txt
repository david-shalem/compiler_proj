# project name
project(compi)

# debug build
set(DCMAKE_BUILD_TYPE, Debug)

# explicitly set compilation flag to create debug info
add_compile_options(--std=c++17 -g -O0)

# reasonably new
cmake_minimum_required(VERSION 3.0)

# header files
include_directories(inc)
include_directories(src)
include(CTest)

# source files
file(GLOB SOURCES "src/*.cpp")

# the invincible flex utility :)
find_package(FLEX)
FLEX_TARGET(
    Lexer
    src/Lexer.lex
    src/Lexer.cpp)

# the invincible bison utility
find_package(BISON)
BISON_TARGET(
    Parser
    src/Parser.y
    src/Parser.cpp
    DEFINES_FILE inc/Parser.h)

# flex - bison connection
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

# single executable file - compi
add_executable(compi ${SOURCES} ${FLEX_Lexer_OUTPUTS} ${BISON_Parser_OUTPUTS})



#tests:

function(test tName tFile expected)
    add_test(NAME "${tName}" COMMAND compi "${tFile}" output.s)
    set_property(TEST "${tName}" PROPERTY PASS_REGULAR_EXPRESSION "${expected}")
endfunction()

#generate test names
function(test_names len keyword)
    if(len GREATER_EQUAL 1)
    foreach(i RANGE 1 "${len}")
        list(APPEND tests "${keyword}_${i}")
        endforeach()
    else()
        set(tests "")
    endif()
    
    
    set(tests "${tests}" PARENT_SCOPE)
endfunction()

#generate list of files and their length
function(get_files keyword)
    file(GLOB files "tests/*_${keyword}.sas")
    list(LENGTH files len)
    set(files "${files}" PARENT_SCOPE)
    set(len "${len}" PARENT_SCOPE)
endfunction()


#simple tests with constant expected output
function(test_group keyword expected)
    get_files("${keyword}")
    test_names("${len}" "${keyword}")
    foreach(file test IN ZIP_LISTS files tests)
        test("${test}" "${file}" "${expected}")
    endforeach()

endfunction()

get_files(out)
test_names("${len}" out)
foreach(file test IN ZIP_LISTS files tests) 
    add_test(NAME "${test}" COMMAND tests/checkFile.py "${file}")
    endforeach()

set(files "")
set(tests "")

test_group(comp "SUCCESS")
test_group(err "FAILED")



