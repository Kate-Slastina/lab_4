cmake_minimum_required(VERSION 3.14)
project(Lab4GUI VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Charts)
qt6_standard_project_setup()

# Самодельные контейнеры
set(MY_SOURCES
    src/my/my_array.hpp
    src/my/my_shared_ptr.hpp
    src/my/my_function.hpp
    src/my/my_random.hpp
    src/my/my_distributions.hpp
)

# Core
set(CORE_SOURCES
    src/core/Sequence.hpp
    src/core/Optional.hpp
    src/core/Pair.hpp
    src/core/Exceptions.hpp
    src/core/DynamicArray.hpp
    src/core/ArraySequence.hpp
    src/core/MutableArraySequence.hpp
)

# Lazy
set(LAZY_SOURCES
    src/lazy/IGenerator.hpp
    src/lazy/LazySequence.hpp
    src/lazy/Iterators.hpp
)

# GUI
set(GUI_SOURCES
    src/gui/mainwindow.cpp
    src/gui/mainwindow.h
    src/gui/mainwindow.ui
    src/gui/generatordialog.cpp
    src/gui/generatordialog.h
    src/gui/generatordialog.ui
    src/gui/value_provider.cpp
    src/gui/value_provider.hpp
    src/main.cpp
)

qt_add_executable(Lab4GUI
    ${MY_SOURCES}
    ${CORE_SOURCES}
    ${LAZY_SOURCES}
    ${GUI_SOURCES}
)

target_include_directories(Lab4GUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lazy
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/my
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(Lab4GUI PRIVATE Qt::Core Qt::Widgets Qt::Charts)

qt_generate_deploy_app_script(TARGET Lab4GUI OUTPUT_SCRIPT deploy_script NO_UNSUPPORTED_PLATFORM_ERROR)
install(SCRIPT ${deploy_script})