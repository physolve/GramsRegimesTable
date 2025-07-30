cmake_minimum_required(VERSION 3.16)

project(example_tableview VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

find_package(Qt6
    REQUIRED COMPONENTS
        Quick
)

qt_add_resources(QML_RESOURCES resources.qrc)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_QML_GENERATE_QMLLS_INI ON)

set(EXECUTABLE_NAME "prototype_table")

project(${EXECUTABLE_NAME} VERSION 1.0.0 LANGUAGES CXX)

qt6_add_executable(${EXECUTABLE_NAME}
    main.cpp
    ${QML_RESOURCES}
) 
qt6_add_qml_module(${EXECUTABLE_NAME}
    URI com.grams.prototable
    VERSION 1.0
    QML_FILES
        MyTablePrototype.qml
        ProtoTable.qml
        ConditionCell.qml
    SOURCES
        prototablemodel.cpp
        prototablemodel.h
        regime.h
)

target_link_libraries(${EXECUTABLE_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Quick
)
