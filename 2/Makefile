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
	cd build && $(call call_cmake)

.PHONY: com_task
com_task: cmake_build
	cd build && cmake --build .
	cp build/main.out main.out

.PHONY: anl_task
anl_task: cmake_build $(OUT_DIR) $(ANAL_DIR)
	$(PYTHON) $(ANAL_BUILD_PATH) --cdb $(OUT_DIR)/compile_commands.json --output $(ANAL_DIR) --use-analyzer clang

.PHONY: run_task
run_task:
	$(call prep_executable, EXEC, ./main.out)
	$(EXEC)

.PHONY: do_tests
do_tests:
	mkdir -p tests/build
	cd tests/build && $(call call_cmake) && cmake --build .
	cp ./tests/build/run_tests.out run_tests.out
	$(call prep_executable, EXEC, ./run_tests.out)
	GCOV_PREFIX_STRIP=69 GCOV_PREFIX="./" $(EXEC)

.PHONY: clear
clear:
	rm -rf build
	rm -rf tests/build
	rm -f main.out
	rm -f run_tests.out
	rm -f test_cocktail.cpp.gcda
	rm -f cocktail.cpp.gcda
