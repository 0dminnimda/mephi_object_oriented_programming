prep_executable = $(eval $(1) := $(shell bash ../../prep_exec.sh $(2)))

.PHONY: fuck
fuck:
	mkdir -p build
	cd build && cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang++"
	cd build && cmake --build .

.PHONY: run
run:
	$(call prep_executable, EXEC, ./build/main)
	$(EXEC) student_work
