OUT_DIR = build
ANAL_DIR = tmp/analysis
CC = clang
CXX = clang++

PYTHON = "$(shell (which python3 2>/dev/null || which python 2>/dev/null) | head -1)"
ANAL_BUILD_PATH = "$(shell which analyze-build)"

prep_executable = $(eval $(1) := $(shell bash ../prep_exec.sh $(2)))

call_cmake = cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="$(CC)" -DCMAKE_CXX_COMPILER="$(CXX)"

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(ANAL_DIR):
	mkdir -p $(ANAL_DIR)

.PHONY: cmake_build
cmake_build: $(OUT_DIR)
	cd $(OUT_DIR) && $(call call_cmake)

.PHONY: compile
compile: cmake_build
	cd $(OUT_DIR) && cmake --build .

.PHONY: analyze
analyze: cmake_build $(OUT_DIR) $(ANAL_DIR)
	$(PYTHON) $(ANAL_BUILD_PATH) --cdb $(OUT_DIR)/compile_commands.json --output $(ANAL_DIR) --use-analyzer clang

.PHONY: run
run:
	$(call prep_executable, EXEC, ./build/main.out)
	$(EXEC)

.PHONY: test
test:
	mkdir -p tests/build
	cd tests/build && $(call call_cmake) && cmake --build .
	$(call prep_executable, EXEC, ./tests/build/run_tests.out)
	GCOV_PREFIX_STRIP=69 GCOV_PREFIX="./" $(EXEC)

.PHONY: clean
clean:
	rm -rf build
	rm -rf tests/build
	rm -f test_cocktail.cpp.gcda
	rm -f cocktail.cpp.gcda
