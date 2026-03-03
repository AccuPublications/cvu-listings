project(Test)

add_library(TestLib SHARED Lib.cpp Lib.h)
target_compile_definitions(TestLib PRIVATE LIB_EXPORT LIB_DEBUG)

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE TestLib)

add_library(TestOdrDll1 SHARED TestOdrDll1.cpp TestOdrDll.h)
target_link_libraries(TestOdrDll1 PRIVATE TestOdrDll2)

add_library(TestOdrDll2 SHARED TestOdrDll2.cpp TestOdrDll.h)

install(TARGETS ${PROJECT_NAME} TestLib TestOdrDll1 TestOdrDll2)
