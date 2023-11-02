# SEE: https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html

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
	cd godot-cpp && git checkout tags/godot-$(GODOT_VERSION)-stable

.PHONY: godot_ext_api
godot_ext_api:
	$(GODOT) --dump-extension-api extension_api.json

.PHONY: build
build:
	# ANDROID_NDK_ROOT=$(HOME)/../usr/ndk scons use_clang_cl=yes use_llvm=yes bits=64 platform=$(PLATFORM)
	# ANDROID_NDK_ROOT=/storage/emulated/0/AndroidSDK/android-ndk-r26b scons use_clang_cl=yes use_llvm=yes bits=64 platform=$(PLATFORM)
	# ANDROID_NDK_ROOT=$(HOME)/../usr/opt/my_sdk/android-ndk-r26b scons use_clang_cl=yes use_llvm=yes bits=64 platform=$(PLATFORM)
	scons use_clang_cl=yes use_llvm=yes bits=64 platform=$(PLATFORM)

.PHONY: build_with_ext_api
build_with_ext_api:
	scons use_clang_cl=yes use_llvm=yes bits=64 custom_api_file=extension_api.json

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
