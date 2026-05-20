PYTHON  := python3
VENV    := venv
BIN     := $(VENV)/bin
PIP     := $(BIN)/pip
PYTEST  := $(BIN)/python -m pytest

.PHONY: all venv build test clean rebuild

all: build

$(VENV)/bin/activate:
	$(PYTHON) -m venv $(VENV)
	$(PIP) install -q --upgrade pip

venv: $(VENV)/bin/activate

build: venv
	$(PIP) install -e .

test: build
	$(PYTEST) tests/ -v

clean:
	rm -rf build/ src/*.egg-info src/**/*.egg-info
	find . -name '*.so' -delete
	find . -name '__pycache__' -exec rm -rf {} +

rebuild: clean build
