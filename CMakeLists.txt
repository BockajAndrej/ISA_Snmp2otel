cmake_minimum_required(VERSION 3.22)
project(snmp2otel)

set(CMAKE_CXX_STANDARD 23)

add_executable(snmpServer tests/snmpAgent.cpp)

add_executable(snmp2otel
        main.cpp
        src/ArgumentParser.cpp
        src/ArgumentParser.h
        src/Structs/SnmpOtelConfig.h
        src/Structs/SnmpOtelErrors.h
        src/SnmpOtelBer.cpp
        src/SnmpOtelBer.h
        src/SnmpOtel_BL.cpp
        src/SnmpOtel_BL.h
        src/Handlers/SignalHandler.cpp
        src/Handlers/SignalHandler.h
        src/SnmpOtelExport.cpp
        src/SnmpOtelExport.h

        libs/BasicSnmp/lib/asn1encoder.cpp
        libs/BasicSnmp/lib/snmpencoder.cpp
)

target_include_directories(snmp2otel PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/nlohmann
)

target_include_directories(snmpServer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)