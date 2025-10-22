cmake_minimum_required(VERSION 3.22)
project(ISA_SNMP2OTEL)

set(CMAKE_CXX_STANDARD 23)

add_executable(snmpServer
        tests/snmpAgent.cpp

        libs/BasicSnmp/lib/asn1encoder.cpp
        libs/BasicSnmp/lib/snmpencoder.cpp
)

add_executable(snmp2otel
        main.cpp
        src/ArgumentParser.cpp
        src/ArgumentParser.h

        src/Structs/SnmpOtelConfig.h
        src/Structs/SnmpOtelErrors.h
        src/Structs/SnmpOtelExportConfig.h

        src/Handlers/SignalHandler.cpp
        src/Handlers/SignalHandler.h

        src/SnmpOtelBer.cpp
        src/SnmpOtelBer.h
        src/SnmpOtel_BL.cpp
        src/SnmpOtel_BL.h

        libs/BasicSnmp/lib/asn1encoder.cpp
        libs/BasicSnmp/lib/snmpencoder.cpp

        src/SnmpOtelJson.cpp
        src/SnmpOtelJson.h
        src/SnmpOtelClient.cpp
        src/SnmpOtelClient.h

        src/SnmpOtelExport.cpp
        src/SnmpOtelExport.h
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