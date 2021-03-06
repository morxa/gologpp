project(golog++)
cmake_minimum_required(VERSION 2.8)

include(CMakeDependentOption)
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(PkgConfig)
find_package(Boost REQUIRED)
find_package(EclipseClp)
find_package(Threads)

pkg_check_modules(SWIPL "swipl")

include_directories(${Boost_INCLUDE_DIR} src)

add_compile_options(-Wall)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -fno-omit-frame-pointer -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2")

add_definitions(-DSOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")

set(EXTRA_COMPILER_MACROS "" CACHE STRING "Extra macro definitions to pass to the compiler with the -D option")

foreach(def ${EXTRA_COMPILER_MACROS})
	message(${def})
	add_definitions(-D${def})
endforeach()

option(USE_LIBASAN "Enable the AddressSanitizer to detect memory management bugs" OFF)
if (${USE_LIBASAN})
	add_compile_options(-fsanitize=address)
endif()


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Options are: Debug Release RelWithDebInfo MinSizeRel."
	FORCE)
endif(NOT CMAKE_BUILD_TYPE)


set(GOLOGPP_VERSION 0.0.1)



######################################################################################
# The golog++ language metamodel: libgolog++.so
######################################################################################

set(INTERFACE_SRC
	src/model/action.cpp
	src/model/variable.cpp
	src/model/value.cpp
	src/model/effect_axiom.cpp
	src/model/fluent.cpp
	src/model/expressions.cpp
	src/model/formula.cpp
	src/model/procedural.cpp
	src/model/scope.cpp
	src/model/utilities.cpp
	src/model/arithmetic.cpp
	src/model/error.cpp
	src/model/user_error.cpp
	src/model/execution.cpp
	src/model/reference.cpp
	src/model/global.cpp
	src/model/domain.cpp
	src/model/string.cpp
	src/model/history.cpp
	src/model/mapping.cpp
	src/model/platform_backend.cpp
	src/model/transition.cpp
	src/model/language.cpp
	src/model/types.cpp
)
add_library(golog++ SHARED ${INTERFACE_SRC})
target_compile_options(golog++ PUBLIC -fPIC)
target_link_libraries (golog++ ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET golog++ PROPERTY CXX_STANDARD 14)
set_property(TARGET golog++ PROPERTY SOVERSION ${GOLOGPP_VERSION})
install(TARGETS golog++ DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES
	src/model/semantics.h
	src/model/formula.h
	src/model/effect_axiom.h
	src/model/variable.h
	src/model/value.h
	src/model/arithmetic.h
	src/model/procedural.h
	src/model/error.h
	src/model/scope.h
	src/model/user_error.h
	src/model/fluent.h
	src/model/action.h
	src/model/execution.h
	src/model/utilities.h
	src/model/language.h
	src/model/global.h
	src/model/reference.h
	src/model/expressions.h
	src/model/gologpp.h
	src/model/domain.h
	src/model/string.h
	src/model/history.h
	src/model/mapping.h
	src/model/platform_backend.h
	src/model/transition.h
	src/model/types.h

	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/golog++/model
)

get_target_property(GOLOGPP_CXXFLAGS_L golog++ COMPILE_OPTIONS)
foreach(i IN LISTS GOLOGPP_CXXFLAGS_L)
	set(GOLOGPP_CXXFLAGS "${GOLOGPP_CXXFLAGS} ${i}")
endforeach()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/golog++.pc.in ${CMAKE_CURRENT_BINARY_DIR}/golog++.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/golog++.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)



######################################################################################
# The golog++ parser: libparsegolog++.so
######################################################################################

option(BUILD_PARSER "Build the parser library" ON)
CMAKE_DEPENDENT_OPTION(DEBUG_PARSER "Enable debug output for the parser" OFF "BUILD_PARSER" OFF)
if (${BUILD_PARSER})
	set(PARSER_SRC
		src/parser/atoms.cpp
		src/parser/action.cpp
		src/parser/arithmetic.cpp
		src/parser/formula.cpp
		src/parser/effect_axiom.cpp
		src/parser/functions.cpp
		src/parser/utilities.cpp
		src/parser/statements.cpp
		src/parser/types.cpp
		src/parser/parser.cpp
		src/parser/assignment.cpp
		src/parser/reference.cpp
		src/parser/fluent.cpp
		src/parser/expressions.cpp
		src/parser/symbolic_expression.cpp
		src/parser/string_expression.cpp
		src/parser/domain.cpp
		src/parser/compound_expression.cpp
		src/parser/field_access.cpp
		src/parser/mapping.cpp
		src/parser/list_expression.cpp
		src/parser/list_access.cpp
	)
	add_library(parsegolog++ SHARED ${PARSER_SRC})
	target_link_libraries(parsegolog++ golog++)
	set_property(TARGET parsegolog++ PROPERTY CXX_STANDARD 14)
	set_property(TARGET parsegolog++ PROPERTY SOVERSION ${GOLOGPP_VERSION})
	target_compile_options(parsegolog++ PRIVATE -Wno-undefined-func-template)
	if (${DEBUG_PARSER})
		target_compile_definitions(parsegolog++ PRIVATE -DBOOST_SPIRIT_DEBUG -DGOLOGPP_DEBUG_PARSER)
	endif()
	install(TARGETS parsegolog++ DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install(FILES
		src/parser/grammar.h
		src/parser/parser.h
		src/parser/formula.h
		src/parser/effect_axiom.h
		src/parser/functions.h
		src/parser/atoms.h
		src/parser/arithmetic.h
		src/parser/statements.h
		src/parser/types.h
		src/parser/fluent.h
		src/parser/action.h
		src/parser/utilities.h
		src/parser/assignment.h
		src/parser/reference.h
		src/parser/expressions.h
		src/parser/symbolic_expression.h
		src/parser/domain.h
		src/parser/compound_expression.h
		src/parser/field_access.h
		src/parser/mapping.h
		src/parser/list_expression.h
		src/parser/list_access.h
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/golog++/parser
	)

	get_target_property(PARSEGOLOGPP_CXXFLAGS_L parsegolog++ COMPILE_OPTIONS)
	foreach(i IN LISTS PARSEGOLOGPP_CXXFLAGS_L)
		set(PARSEGOLOGPP_CXXFLAGS "${PARSEGOLOGPP_CXXFLAGS} ${i}")
	endforeach()
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/parsegolog++.pc.in ${CMAKE_CURRENT_BINARY_DIR}/parsegolog++.pc @ONLY)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/parsegolog++.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()



######################################################################################
# The golog++ readylog semantics: libreadylog++.so
######################################################################################

if (${ECLIPSE_FOUND})
	option(BUILD_READYLOG_IMPL "Build the readylog semantics (ECLiPSe Prolog)" ON)
endif()
if (${BUILD_READYLOG_IMPL})
	find_file(READYLOG_PL preprocessor.pl
		PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../
		PATH_SUFFIXES /readylog/interpreter /kbsgolog/interpreters/readylog)

	set(READYLOG_SRC
		src/semantics/readylog/action.cpp
		src/semantics/readylog/variable.cpp
		src/semantics/readylog/value.cpp
		src/semantics/readylog/domain.cpp
		src/semantics/readylog/arithmetic.cpp
		src/semantics/readylog/execution.cpp
		src/semantics/readylog/fluent.cpp
		src/semantics/readylog/formula.cpp
		src/semantics/readylog/procedural.cpp
		src/semantics/readylog/scope.cpp
		src/semantics/readylog/semantics.cpp
		src/semantics/readylog/utilities.cpp
		src/semantics/readylog/string.cpp
		src/semantics/readylog/history.cpp
		src/semantics/readylog/reference.cpp
		src/semantics/readylog/effect_axiom.cpp
	)

	link_directories(${ECLIPSE_LIBRARY_DIRS})
	add_library(readylog++ SHARED ${READYLOG_SRC})
	target_compile_definitions(readylog++ PRIVATE -DREADYLOG_PATH=\"${READYLOG_PL}\")
	target_compile_definitions(readylog++ PUBLIC -DECLIPSE_DIR=\"${ECLIPSE_DIR}\" -DUSES_NO_ENGINE_HANDLE)
	target_include_directories(readylog++ PUBLIC ${ECLIPSE_INCLUDE_DIRS})
	target_link_libraries(readylog++ golog++ ${ECLIPSE_LIBRARIES})
	set_property(TARGET readylog++ PROPERTY CXX_STANDARD 14)
	set_property(TARGET readylog++ PROPERTY SOVERSION ${GOLOGPP_VERSION})
	install(TARGETS readylog++ DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install(FILES
		src/semantics/readylog/semantics.h
		src/semantics/readylog/formula.h
		src/semantics/readylog/effect_axiom.h
		src/semantics/readylog/variable.h
		src/semantics/readylog/value.h
		src/semantics/readylog/domain.h
		src/semantics/readylog/arithmetic.h
		src/semantics/readylog/procedural.h
		src/semantics/readylog/scope.h
		src/semantics/readylog/fluent.h
		src/semantics/readylog/action.h
		src/semantics/readylog/execution.h
		src/semantics/readylog/utilities.h
		src/semantics/readylog/reference.h
		src/semantics/readylog/config.h
		src/semantics/readylog/string.h
		src/semantics/readylog/history.h
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/golog++/semantics/readylog
	)

	get_target_property(READYLOGPP_CXXFLAGS_L readylog++ COMPILE_OPTIONS)
	set(READYLOGPP_CXXFLAGS "-I${ECLIPSE_INCLUDE_DIRS}")
	foreach(i IN LISTS ${READYLOGPP_CXXFLAGS_L})
		set(READYLOGPP_CXXFLAGS "${READYLOGPP_CXXFLAGS} ${i}")
	endforeach()
	set(READYLOGPP_LDFLAGS "${ECLIPSE_LDFLAGS}")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/readylog++.pc.in ${CMAKE_CURRENT_BINARY_DIR}/readylog++.pc @ONLY)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/readylog++.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()



######################################################################################
# Tests
######################################################################################

option(BUILD_TESTS "Build tests" ON)

CMAKE_DEPENDENT_OPTION(TEST_OBJECTMODEL "Enable test for object model" ON "BUILD_TESTS" OFF)
CMAKE_DEPENDENT_OPTION(TEST_PARSER "Enable test for parser" ON "BUILD_TESTS;BUILD_PARSER" OFF)
CMAKE_DEPENDENT_OPTION(TEST_ECLIPSE "Enable test for eclipse-clp" OFF "BUILD_TESTS;ECLIPSE_FOUND" OFF)
CMAKE_DEPENDENT_OPTION(TEST_READYLOG "Try running test programs in readylog" ON "BUILD_TESTS;ECLIPSE_FOUND;BUILD_READYLOG_IMPL" OFF)

if (${BUILD_TESTS})
	set(TEST_SRC
		src/tests/readylog-test.cpp)
	add_executable(readylog-test ${TEST_SRC})
	set(TEST_LINK_LIBRARIES ${TEST_LINK_LIBRARIES} golog++)

	if (${TEST_OBJECTMODEL})
		target_compile_definitions(readylog-test PRIVATE -DGOLOGPP_TEST_OBJECTMODEL)
	endif()

	if (${TEST_PARSER})
		target_compile_definitions(readylog-test PRIVATE -DGOLOGPP_TEST_PARSER)
		set(TEST_LINK_LIBRARIES ${TEST_LINK_LIBRARIES} parsegolog++)
	endif()

	if (${TEST_ECLIPSE})
		target_compile_definitions(readylog-test PRIVATE -DGOLOGPP_TEST_ECLIPSE)
		set(TEST_LINK_LIBRARIES ${TEST_LINK_LIBRARIES} ${ECLIPSE_LIBRARIES})
	endif()

	if (${TEST_READYLOG})
		target_compile_definitions(readylog-test PRIVATE -DGOLOGPP_TEST_READYLOG)
		set(TEST_LINK_LIBRARIES ${TEST_LINK_LIBRARIES} readylog++)
	endif()


	if (USE_LIBASAN)
		target_link_libraries(readylog-test asan)
	endif()
	target_link_libraries(readylog-test ${TEST_LINK_LIBRARIES})
	set_property(TARGET readylog-test PROPERTY CXX_STANDARD 14)
endif()

