project(Example)

add_executable(${PROJECT_NAME} Main.cpp GetData.cpp)

add_library(GetData SHARED GetData.cpp)
target_compile_definitions(GetData PRIVATE LIB_EXPORT)

add_executable(SharedExample Main.cpp)
target_link_libraries(SharedExample PRIVATE GetData)

install(TARGETS GetData SharedExample)
