# SEE: https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html

GODOT = "$(shell godot_path.sh)" --rendering-driver opengl3
GODOT_VERSION = "4.1.1"
PYTHON = "$(shell (which python3 2>/dev/null || which python 2>/dev/null) | head -1)"
PLATFORM = "$(shell $(PYTHON) -c 'import platform; print(platform.system().lower())')"
NDK_ROOT = ""

.PHONY: all
all:
	:

.PHONY: setup_godot_cpp_uuhh
setup_godot_cpp_uuhh:
	git submodule init
	git submodule update
	cd godot-cpp && git fetch
	cd godot-cpp && git checkout tags/godot-$(GODOT_VERSION)-stable
	mkdir -p gdextension
	cp -f godot-cpp/gdextension/extension_api.json gdextension/extension_api.json
	cp -f godot-cpp/gdextension/gdextension_interface.h gdextension/gdextension_interface.h
	cd godot-cpp && git checkout master
	cd godot-cpp && git pull

.PHONY: setup_godot_cpp_eeck
setup_godot_cpp_eeck:
	cd godot-cpp && git reset --hard
	mkdir -p godot-cpp-save
	git submodule init
	git submodule update
	cd godot-cpp && git fetch
	cd godot-cpp && git checkout master
	cd godot-cpp && git pull
	cp -f -r godot-cpp/tools godot-cpp-save/tools
	cp -f godot-cpp/SConstruct godot-cpp-save/SConstruct
	cd godot-cpp && git checkout tags/godot-$(GODOT_VERSION)-stable
	mkdir -p gdextension
	cp -f godot-cpp/gdextension/extension_api.json gdextension/extension_api.json
	cp -f godot-cpp/gdextension/gdextension_interface.h gdextension/gdextension_interface.h
	cp -f -r godot-cpp-save/tools godot-cpp/tools
	cp -f godot-cpp-save/SConstruct godot-cpp/SConstruct

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
	ANDROID_NDK_ROOT=$(NDK_ROOT) scons platform=$(PLATFORM) use_clang_cl=yes use_llvm=yes compiledb=yes ANDROID_HOME="" gdextension_dir="gdextension"
	- mv demo/bin/lab3.linux.template_debug.x86_64.so demo/bin/liblab3.linux.template_debug.x86_64.so

.PHONY: build_no_ext_api
build_no_ext_api:
	ANDROID_NDK_ROOT=$(NDK_ROOT) scons platform=$(PLATFORM) use_clang_cl=yes use_llvm=yes compiledb=yes ANDROID_HOME=""

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

.PHONY: uml
uml:
	hpp2plantuml -i design.cpp -o design.puml
	hpp2plantuml -i design.cpp -d -o design_dep.puml
	python fix_puml.py design.puml
	python fix_puml.py design_dep.puml
