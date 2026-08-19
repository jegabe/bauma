cmake_minimum_required(VERSION 3.16)

project(bauma LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB HEADER_FILES "src/bma/*.h")
foreach(HEADER IN LISTS HEADER_FILES)
	set_source_files_properties(${HEADER} PROPERTIES LANGUAGE C)
	if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
		set_source_files_properties(${HEADER} PROPERTIES COMPILE_OPTIONS "-x;c")
	elseif(MSVC)
		set_source_files_properties(${HEADER} PROPERTIES COMPILE_OPTIONS "/TC")
	endif()
endforeach()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
	add_compile_options(-Wall -Wextra)
endif()

include_directories(src)

add_executable(test_ccal src/bma/ccal.h)
target_compile_definitions(test_ccal PRIVATE BMA_CCAL_IMPL BMA_CCAL_TEST)

add_executable(test_cmdln src/bma/cmdln.h)
target_compile_definitions(test_cmdln PRIVATE BMA_CCAL_IMPL BMA_CMDLN_IMPL BMA_CMDLN_TEST)

add_executable(test_cmdsl src/bma/cmdsl.h)
target_compile_definitions(test_cmdsl PRIVATE BMA_CCAL_IMPL BMA_CMDSL_IMPL BMA_CMDSL_TEST)

add_executable(test_fs src/bma/fs.h)
target_compile_definitions(test_fs PRIVATE BMA_CCAL_IMPL BMA_UTILS_IMPL BMA_FS_IMPL BMA_FS_TEST)

add_executable(test_log src/bma/log.h)
target_compile_definitions(test_log PRIVATE BMA_CCAL_IMPL BMA_THRD_IMPL BMA_LOG_IMPL BMA_LOG_TEST)

add_executable(test_nson src/bma/nson.h)
target_compile_definitions(test_nson PRIVATE BMA_CCAL_IMPL BMA_NSON_IMPL BMA_NSON_TEST)

add_executable(test_thrd src/bma/thrd.h)
target_compile_definitions(test_thrd PRIVATE BMA_CCAL_IMPL BMA_THRD_IMPL BMA_THRD_TEST)

add_executable(test_utils src/bma/utils.h)
target_compile_definitions(test_utils PRIVATE BMA_CCAL_IMPL BMA_UTILS_IMPL BMA_UTILS_TEST)

enable_testing()

add_test(NAME test_ccal COMMAND test_ccal)
add_test(NAME test_cmdln COMMAND test_cmdln)
add_test(NAME test_cmdsl COMMAND test_cmdsl)
add_test(NAME test_fs COMMAND test_fs)
add_test(NAME test_log COMMAND test_log)
add_test(NAME test_nson COMMAND test_nson)
add_test(NAME test_thrd COMMAND test_thrd)
add_test(NAME test_utils COMMAND test_utils)
