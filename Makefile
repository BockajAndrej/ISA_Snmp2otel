BUILD_DIR := build
TARGET_EXECUTABLE := snmp2otel
TEST_EXECUTABLE := otelJsonUnitTests

.PHONY: all build run test clean setup

setup:
	@echo "Config CMake..."
	@mkdir -p $(BUILD_DIR)
	@(cd $(BUILD_DIR) && cmake ..)

build: setup
	@echo "Start compilation (make)..."
	@(cd $(BUILD_DIR) && make -j4)

all: build test run 

run:
	@echo "Start main program (testing): ./$(BUILD_DIR)/$(TARGET_EXECUTABLE)"
	./$(BUILD_DIR)/$(TARGET_EXECUTABLE) -t 127.0.0.1 -e http://localhost:4318/v1/metrics -o ./Data/Inputs/oids_file.txt

test:
	@echo "Start tests program: ./$(BUILD_DIR)/$(TEST_EXECUTABLE)"
	./$(BUILD_DIR)/$(TEST_EXECUTABLE)

clean:
	@echo "Remove build directory$(BUILD_DIR)"
	@rm -rf $(BUILD_DIR)