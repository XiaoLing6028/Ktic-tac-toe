# SPDX-License-Identifier: AGPL-3.0-or-later
cmake_minimum_required(VERSION 3.21)

project(ktictactoe VERSION 1.0.0 LANGUAGES CXX)

set(APP_ID "io.github.XiaoLing6028.Ktictactoe" CACHE STRING "Reverse-DNS application ID")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

include(GNUInstallDirs)
include(CTest) # provides BUILD_TESTING (ON by default)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Svg)

# --- Pure game logic (no Qt) -------------------------------------------------
add_library(ktictactoe_core STATIC src/board.cpp src/bot.cpp)
target_include_directories(ktictactoe_core PUBLIC src)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ktictactoe_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# --- Application ---------------------------------------------------------------
qt_add_executable(ktictactoe
    src/main.cpp
    src/mainwindow.cpp src/mainwindow.h
    src/boardwidget.cpp src/boardwidget.h
    src/theme.h
)
qt_add_resources(ktictactoe "icons"
    PREFIX "/icons"
    BASE data/icons
    FILES data/icons/ktictactoe.svg
)
target_link_libraries(ktictactoe PRIVATE ktictactoe_core Qt6::Widgets)
target_compile_definitions(ktictactoe PRIVATE
    KTICTACTOE_VERSION="${PROJECT_VERSION}"
    KTICTACTOE_APP_ID="${APP_ID}"
    QT_NO_CAST_FROM_ASCII QT_NO_KEYWORDS
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ktictactoe PRIVATE -Wall -Wextra)
endif()

install(TARGETS ktictactoe RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# --- Desktop integration (freedesktop / KDE standard paths) -------------------
install(FILES data/${APP_ID}.desktop      DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES data/${APP_ID}.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)

install(FILES data/icons/ktictactoe.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        RENAME ${APP_ID}.svg)
foreach(size 16 22 32 48 64 128 256)
    install(FILES data/icons/ktictactoe-${size}.png
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${size}x${size}/apps
            RENAME ${APP_ID}.png)
endforeach()

# --- Tests ---------------------------------------------------------------------
if(BUILD_TESTING)
    add_executable(test_logic tests/test_logic.cpp)
    target_link_libraries(test_logic PRIVATE ktictactoe_core)
    add_test(NAME logic COMMAND test_logic)

    find_program(DESKTOP_FILE_VALIDATE desktop-file-validate)
    if(DESKTOP_FILE_VALIDATE)
        add_test(NAME desktop-file COMMAND ${DESKTOP_FILE_VALIDATE}
                 ${CMAKE_CURRENT_SOURCE_DIR}/data/${APP_ID}.desktop)
    endif()
    find_program(APPSTREAMCLI appstreamcli)
    if(APPSTREAMCLI)
        add_test(NAME metainfo COMMAND ${APPSTREAMCLI} validate --no-net --pedantic --explain
                 ${CMAKE_CURRENT_SOURCE_DIR}/data/${APP_ID}.metainfo.xml)
    endif()
endif()
