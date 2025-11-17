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
	@(cd $(BUILD_DIR) && make -j$(shell nproc))

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


startOtelCollector:
	@echo "Start otel-collector..."
	sudo docker run -d --name otel-collector-test -v $(pwd)/tests/OTEL/collector-config.yaml:/etc/otel-collector-contrib/config.yaml -p 4318:4318 otel/opentelemetry-collector-contrib:latest

stopOtelCollector:
	@echo "Stop otel-collector..."
	sudo docker stop otel-collector-test

clearOtelCollector:
	@echo "Remove otel-collector..."
	sudo docker rm otel-collector-test

logOtel:
	@echo "LOGS of received metrics: "
	sudo docker logs otel-collector-test