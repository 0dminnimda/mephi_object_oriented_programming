OUT_DIR = build
ANAL_DIR = analysis
TEST_DIR = tests/build
COV_DIR = tests/coverage

CC = clang
CXX = clang++

PYTHON = "$(shell (which python3 2>/dev/null || which python 2>/dev/null) | head -1)"
ANAL_BUILD_PATH = "$(shell which analyze-build)"

prep_executable = $(eval $(1) := $(shell bash ../prep_exec.sh $(2) --use_dir))

call_cmake = cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="$(CC)" -DCMAKE_CXX_COMPILER="$(CXX)"

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(ANAL_DIR):
	mkdir -p $(ANAL_DIR)

$(COV_DIR):
	mkdir -p $(COV_DIR)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

.PHONY: cmake_build
cmake_build: $(OUT_DIR)
	cd $(OUT_DIR) && $(call call_cmake)

.PHONY: setup_sfml
setup_sfml:
	git submodule init
	git submodule update
	cd ../SFML && git checkout 2.6.x

.PHONY: compile
compile: cmake_build
	cd $(OUT_DIR) && cmake --build .

.PHONY: analyze
analyze: cmake_build $(OUT_DIR) $(ANAL_DIR)
	$(PYTHON) $(ANAL_BUILD_PATH) --cdb $(OUT_DIR)/compile_commands.json --output $(ANAL_DIR) --use-analyzer clang

# https://wiki.termux.com/wiki/Graphical_Environment
.PHONY: setup_termux_gui
setup_termux_gui:
	vncserver :1 -localhost

.PHONY: run
run:
	$(call prep_executable, EXEC, ./build/bin/main.out)
	DISPLAY=":1" $(EXEC)

llvm_cov = llvm-cov $(1) $(2) -instr-profile=$(3) --ignore-filename-regex="tests/*"

.PHONY: test
test: $(TEST_DIR) $(COV_DIR)
	cd $(TEST_DIR) && $(call call_cmake) && cmake --build .
	$(call prep_executable, EXEC, $(TEST_DIR)/run_tests.out)
	LLVM_PROFILE_FILE="$(COV_DIR)/run_tests.profraw" $(EXEC)
	llvm-profdata merge -sparse $(COV_DIR)/run_tests.profraw -o $(COV_DIR)/merged.profdata
	$(call llvm_cov,report,$(EXEC),$(COV_DIR)/merged.profdata)
	$(call llvm_cov,show,$(EXEC),$(COV_DIR)/merged.profdata) > $(COV_DIR)/report.txt

.PHONY: docs
docs:
	doxygen Doxyfile

.PHONY: clean
clean:
	rm -rf $(OUT_DIR)
	rm -rf $(ANAL_DIR)
	rm -rf $(TEST_DIR)
	rm -rf $(COV_DIR)
