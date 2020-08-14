cmake_minimum_required(VERSION 2.6)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../../lib)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../../lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../../lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../../lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/../bin)
#set(CMAKE_DEBUG_POSTFIX d)

project(LP)

##配置QT路径
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    #list(APPEND CMAKE_PREFIX_PATH "${QT}/msvc2015")
    set(PROJECT_PLATFORM "Win32")
    set(PROJECT_PLATFORM_X "X86")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    #list(APPEND CMAKE_PREFIX_PATH "${QT}/msvc2015_64")
    set(PROJECT_PLATFORM "x64")
    set(PROJECT_PLATFORM_X "X64")
endif()

include_directories(
	${PROJECT_SOURCE_DIR}/../thirdparty/tinyxml
	${PROJECT_SOURCE_DIR}/../thirdparty/cJSON
	D:/GMP/externals/GBIM/include
)
link_directories(
	${PROJECT_SOURCE_DIR}/../lib
	D:/GMP/externals/GBIM/Lib/${PROJECT_PLATFORM}/$<CONFIG>
)

FILE(GLOB SRC_FILES "./*.c*") 
FILE(GLOB HEAD_FILES "./*.h") 

add_compile_definitions("TIXML_USE_STL" "_CRT_SECURE_NO_DEPRECATE")

#创建工程文件
add_executable(${PROJECT_NAME} ${SRC_FILES} ${HEAD_FILES})

#set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX d)

#target_link_libraries(${PROJECT_NAME} debug tinyxmld optimized tinyxml)
target_link_libraries(${PROJECT_NAME} debug cjsond optimized cjson)
target_link_libraries(${PROJECT_NAME} debug commond optimized common)