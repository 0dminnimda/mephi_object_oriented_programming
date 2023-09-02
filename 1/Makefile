.PHONY: task
task:
	clang main.cpp -o main.out -lstdc++ -std=c++17 -fsanitize=address

.PHONY: run_task
run_task:
	bash ../exec.sh main.out
