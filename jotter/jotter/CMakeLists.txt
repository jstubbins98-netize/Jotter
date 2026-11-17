cmake_minimum_required(VERSION 3.28)
project(Jotter VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 6.8 REQUIRED COMPONENTS Widgets Gui)
find_package(OpenSSL 3 REQUIRED COMPONENTS Crypto)
find_package(libzip CONFIG QUIET)
if(TARGET libzip::zip)
    set(JOTTER_ZIP_TARGET libzip::zip)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBZIP REQUIRED IMPORTED_TARGET libzip)
    set(JOTTER_ZIP_TARGET PkgConfig::LIBZIP)
endif()

qt_add_executable(jotter WIN32
    src/main.cpp src/MainWindow.cpp src/MainWindow.h
    src/Journal.cpp src/Journal.h
    src/Crypto.cpp src/Crypto.h
    src/Document.cpp src/Document.h
    src/Export.cpp src/Export.h
)
target_link_libraries(jotter PRIVATE Qt6::Widgets Qt6::Gui OpenSSL::Crypto ${JOTTER_ZIP_TARGET})
if(MSVC)
    target_compile_options(jotter PRIVATE /W4 /permissive- /utf-8)
    target_compile_definitions(jotter PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
else()
    target_compile_options(jotter PRIVATE -Wall -Wextra -Wpedantic)
endif()
include(GNUInstallDirs)
install(TARGETS jotter RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
qt_generate_deploy_app_script(TARGET jotter OUTPUT_SCRIPT deploy_script NO_UNSUPPORTED_PLATFORM_ERROR)
install(SCRIPT ${deploy_script})
