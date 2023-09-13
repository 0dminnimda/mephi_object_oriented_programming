OUT_DIR = build
CC = clang
CXX = clang++

prep_executable = $(eval $(1) := $(shell bash ../prep_exec.sh $(2)))

.PHONY: com_task
com_task: $(OUT_DIR)
	cd build && cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="${CC}" -DCMAKE_CXX_COMPILER="${CXX}" && cmake --build .
	cp build/main.out main.out

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

.PHONY: run_task
run_task:
	$(call prep_executable, EXEC, ./main.out)
	$(EXEC)

.PHONY: clear
clear:
	rm -rf build
