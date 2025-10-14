cmake_minimum_required(VERSION 3.22)
project(snmp2otel)

set(CMAKE_CXX_STANDARD 23)

add_executable(snmpServer tests/snmpAgent.cpp)

add_executable(snmp2otel main.cpp
        src/ArgumentParser.cpp
        src/ArgumentParser.h
        src/Structs/SnmpOtelConfig.h
        src/Structs/SnmpOtelErrors.h
        src/SnmpOtelBer.cpp
        src/SnmpOtelBer.h
        src/SnmpOtel_BL.cpp
        src/SnmpOtel_BL.h
)