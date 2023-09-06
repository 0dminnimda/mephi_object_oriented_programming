OUT_DIR = build
ANAL_DIR = tmp/analysis
CC = clang
CXX = clang++

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(ANAL_DIR):
	mkdir -p $(ANAL_DIR)

.PHONY: cmake_build
cmake_build: $(OUT_DIR)
	cd build && cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="$(CC)" -DCMAKE_CXX_COMPILER="$(CXX)"

.PHONY: com_task
com_task: cmake_build
	cd build && cmake --build .
	cp build/main.out main.out

.PHONY: anl_task
anl_task: cmake_build $(OUT_DIR) $(ANAL_DIR)
	analyze-build --cdb $(OUT_DIR)/compile_commands.json --output $(ANAL_DIR) --use-analyzer clang

.PHONY: run_task
run_task:
	bash ../exec.sh main.out

.PHONY: clear
clear:
	rm -rf build
