PYTHON = "$(shell (which python3 2>/dev/null || which python 2>/dev/null) | head -1)"

.PHONY: all
all:
	:

.PHONY: uml
uml:
	hpp2plantuml -i src/game.cpp -o design.puml
	$(PYTHON) fix_puml.py design.puml
