run_scons = scons platform=windows use_clang_cl=yes use_llvm=yes bits=64
GODOT = $(godot_path.sh)

.PHONY: all
all:
	:

.PHONY: setup
setup:
setup:
	$GODOT --dump-extension-api extension_api.json --rendering-driver opengl3
	cd godot-cpp && $(call run_scons) custom_api_file=../extension_api.json

.PHONY: build
build:
build:
	$(call run_scons) custom_api_file=extension_api.json

# mnt/c/Godot/Godot_v4.1.2-stable_linux.x86_64/Godot_v4.1.2-stable_linux.x86_64
# C:\Godot\Godot_v4.1.1-stable_win64\Godot_v4.1.1-stable_win64.exe --rendering-driver opengl3
# C:\Godot\Godot_v4.1.1-stable_win64\Godot_v4.1.1-stable_win64_console.exe 
