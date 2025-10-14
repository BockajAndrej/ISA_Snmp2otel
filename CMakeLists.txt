cmake_minimum_required(VERSION 3.22)
project(snmp2otel)

set(CMAKE_CXX_STANDARD 23)

add_executable(snmpServer tests/snmpAgent.cpp)

add_executable(snmp2otel main.cpp
        src/Snmp2otel_BL.cpp
        src/Snmp2otel_BL.h
        src/snmpBer.cpp
        src/snmpBer.h
        src/InputParser.cpp
        src/InputParser.h
)