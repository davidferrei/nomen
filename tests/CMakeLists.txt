enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Test Core Gui Widgets)
find_package(Threads REQUIRED)

file(GLOB_RECURSE PROJECT_SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.h
)

list(FILTER PROJECT_SOURCES EXCLUDE REGEX ".*/main\\.cpp$|.*\\.ui$|.*\\.qrc$")

function(add_test_executable TEST_NAME)

    add_executable(${TEST_NAME}
        ${TEST_NAME}.cpp
        ${PROJECT_SOURCES}
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        Qt6::Test Qt6::Core Qt6::Gui Qt6::Widgets
        ibpp lmdb ${SLEUTHKIT_LIB} Threads::Threads
        ${CMAKE_SOURCE_DIR}/lib/ibpp/libfbclient.so
    )

    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/lib
        ${SLEUTHKIT_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/lib/ibpp/core
        ${CMAKE_SOURCE_DIR}/lib/liblmdb
    )

    target_compile_definitions(${TEST_NAME} PRIVATE IBPP_LINUX)
    target_compile_features(${TEST_NAME} PRIVATE cxx_std_17)
    set_target_properties(${TEST_NAME} PROPERTIES BUILD_RPATH "$ORIGIN/../lib/ibpp")
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    
endfunction()

# Test List
add_test_executable(tst_caseManager)
add_test_executable(tst_indexDisk)
