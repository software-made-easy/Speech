cmake_minimum_required(VERSION 3.12)

project(Speech VERSION 1.4.3 LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools TextToSpeech)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools TextToSpeech)

if(UNIX AND NOT APPLE AND NOT ANDROID)
    set(LINUX TRUE)
endif()

set(TS_FILES translations/Speech_de_DE.ts)
set(RC_FILES
    translations/translations.qrc
)

include(cmake/TranslationUtils.cmake)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        ui/mainwindow.ui
        ${RC_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_translation(QM_FILES ${TS_FILES})
    add_app_translations_resource(APP_RES ${QM_FILES})

    qt_add_executable(speech
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${APP_RES}
    )

    set_property(TARGET speech APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
                 ${CMAKE_CURRENT_SOURCE_DIR}/android)

else()
    qt5_add_translation(QM_FILES ${TS_FILES})
    add_app_translations_resource(APP_RES ${QM_FILES})

    if(ANDROID)
        add_library(speech SHARED
            ${PROJECT_SOURCES}
            ${APP_RES}
        )

        set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")

    else()
        add_executable(speech
            ${PROJECT_SOURCES}
            ${APP_RES}
        )
    endif()
endif()

IF(WIN32)
    SET(INTL_LDFLAGS -lintl)
ENDIF(WIN32)

target_include_directories(speech PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(speech PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::TextToSpeech ${INTL_LDFLAGS})

target_compile_definitions(speech PUBLIC APP_VERSION="${PROJECT_VERSION}" QT_DISABLE_DEPRECATED_BEFORE=0x051208 CHECK_MARKDOWN)

set_target_properties(speech PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER org.qtproject.speech
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(speech)
endif()

if (LINUX)
    include(GNUInstallDirs)
    install(TARGETS speech DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
