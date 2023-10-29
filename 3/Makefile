GODOT = $(shell godot_path.sh)

.PHONY: all
all:
	:

.PHONY: setup
setup:
	# maybe cd godot-cpp && git checkout godot-4.1.1-stable
	$(GODOT) --dump-extension-api extension_api.json --rendering-driver opengl3
	# $(GODOT) --dump-extension-api extension_api.json

.PHONY: build
build:
	scons platform=windows use_clang_cl=yes use_llvm=yes bits=64 custom_api_file=extension_api.json

.PHONY: run
run:
	$(GODOT) --rendering-driver opengl3

# mnt/c/Godot/Godot_v4.1.2-stable_linux.x86_64/Godot_v4.1.2-stable_linux.x86_64
# C:\Godot\Godot_v4.1.1-stable_win64\Godot_v4.1.1-stable_win64.exe --rendering-driver opengl3
# C:\Godot\Godot_v4.1.1-stable_win64\Godot_v4.1.1-stable_win64_console.exe 
# https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html#doc-gdextension-cpp-example
