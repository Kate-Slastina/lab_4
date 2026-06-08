cmake_minimum_required(VERSION 3.14)
project(Lab4 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

set(CORE_SOURCES
    src/core/Sequence.hpp
    src/core/Optional.hpp
    src/core/Pair.hpp
    src/core/Exceptions.hpp
    src/core/DynamicArray.hpp
    src/core/ArraySequence.hpp
    src/core/MutableArraySequence.hpp
)

set(LAZY_SOURCES
    src/lazy/LazySequence.hpp
    src/lazy/IGenerator.hpp
    src/lazy/DefaultGenerator.hpp
    src/lazy/SkipGenerator.hpp
    src/lazy/ConcatGenerator.hpp
    src/lazy/Iterators.hpp
)

set(STREAM_SOURCES
    src/stream/Stream.hpp
    src/stream/ReadOnlyStream.hpp
    src/stream/FileStream.hpp
    src/stream/FileStream.cpp
    src/stream/Deserializer.hpp
    src/stream/Deserializer.cpp
)

set(TESTDATA_SOURCES
    src/testdata/SmartTestDataGenerator.hpp
    src/testdata/SmartTestDataGenerator.cpp
    src/testdata/Distribution.hpp
    src/testdata/SequenceProperty.hpp
    src/testdata/Generators/LinearTrendGenerator.hpp
    src/testdata/Generators/RandomWalkGenerator.hpp
    src/testdata/Generators/PeriodicGenerator.hpp
    src/testdata/Generators/CompositeGenerator.hpp
)

add_library(Lab4Core STATIC
    ${CORE_SOURCES}
    ${LAZY_SOURCES}
    ${STREAM_SOURCES}
    ${TESTDATA_SOURCES}
)

target_include_directories(Lab4Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lazy
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream
    ${CMAKE_CURRENT_SOURCE_DIR}/src/testdata
)

enable_testing()
find_package(GTest REQUIRED)

add_executable(Lab4Tests
    tests/test_lazy_sequence.cpp
    tests/test_stream.cpp
    tests/test_smart_generator.cpp
    tests/test_predefined_data.cpp
    tests/test_large_scale.cpp
)

target_link_libraries(Lab4Tests
    Lab4Core
    GTest::gtest
    GTest::gtest_main
)

target_include_directories(Lab4Tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

add_test(NAME Lab4Tests COMMAND Lab4Tests)