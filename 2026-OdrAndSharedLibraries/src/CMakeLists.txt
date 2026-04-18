project(OdrCheck CXX)

if(MSVC)
# We need the install directory to find the DIA SDK
if("$ENV{VSINSTALLDIR}" STREQUAL "")
  string(FIND "${CMAKE_CXX_COMPILER}" "/VC/" VC_INDEX REVERSE)
  if(${VC_INDEX} EQUAL "-1")
    message(FATAL_ERROR "Please run from a Visual Studio command prompt")
  endif()
  string(SUBSTRING "${CMAKE_CXX_COMPILER}" 0 ${VC_INDEX} VSINSTALLDIR)
  set(ENV{VSINSTALLDIR} "${VSINSTALLDIR}")
endif()
endif()

if(MSVC)
add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp ${PROJECT_NAME}.h WinTypeReader.cpp WinTypeReader.h)
target_include_directories(${PROJECT_NAME} PRIVATE $ENV{VSINSTALLDIR})
target_link_options(${PROJECT_NAME} PRIVATE setargv.obj)
else()
add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp ${PROJECT_NAME}.h LinuxTypeReader.cpp LinuxTypeReader.h)
target_link_libraries(${PROJECT_NAME} dw)
endif()

install(TARGETS ${PROJECT_NAME})
