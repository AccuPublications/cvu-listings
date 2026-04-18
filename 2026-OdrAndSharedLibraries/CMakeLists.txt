cmake_minimum_required(VERSION 3.20)

project(ODRArticle CXX)

set(CMAKE_CXX_STANDARD 23)

if(MSVC)
add_compile_options(/W4 /WX)
else()
add_compile_options(-ggdb -Wall -Wextra -flto -fvisibility=hidden)
endif()

add_subdirectory(example)
add_subdirectory(src)
add_subdirectory(test)
