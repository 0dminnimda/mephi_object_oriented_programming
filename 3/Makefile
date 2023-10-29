GODOT = "$(shell godot_path.sh)" --rendering-driver opengl3
GODOT_VERSION = "4.1.1"

.PHONY: all
all:
	:

.PHONY: setup_godot_cpp
setup_godot_cpp:
	git submodule init
	git submodule update
	cd godot-cpp && git fetch
	cd godot-cpp && git checkout tag/godot-$(GODOT_VERSION)-stable

.PHONY: godot_ext_api
godot_ext_api:
	$(GODOT) --dump-extension-api extension_api.json

.PHONY: build
build:
	scons platform=windows use_clang_cl=yes use_llvm=yes bits=64
#	custom_api_file=extension_api.json

.PHONY: run
run:
	$(GODOT)

.PHONY: build_game
build_game:
	mkdir -p demo/build
	$(GODOT) --path demo --export-debug "Windows Desktop" build/game.exe

.PHONY: run_game
run_game:
	demo/build/game.exe --rendering-driver opengl3

# mnt/c/Godot/Godot_v4.1.2-stable_linux.x86_64/Godot_v4.1.2-stable_linux.x86_64
# C:\Godot\Godot_v4.1.1-stable_win64\Godot_v4.1.1-stable_win64.exe --rendering-driver opengl3
# C:\Godot\Godot_v4.1.1-stable_win64\Godot_v4.1.1-stable_win64_console.exe 
# https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html#doc-gdextension-cpp-example
