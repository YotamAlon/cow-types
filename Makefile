PYTHON  := python3
VENV    := venv
BIN     := $(VENV)/bin
PIP     := $(BIN)/pip
PYTEST  := $(BIN)/python -m pytest

.PHONY: all venv build test clean rebuild compile-commands

all: build

$(VENV)/bin/activate:
	$(PYTHON) -m venv $(VENV)
	$(PIP) install -q --upgrade pip

venv: $(VENV)/bin/activate

build: venv
	$(PIP) install -e .

test: build
	$(PYTEST) tests/ -v

compile-commands:
	python3 -c "import json,os,sysconfig,subprocess;inc=sysconfig.get_path('include');sdk=subprocess.check_output(['xcrun','--show-sdk-path'],text=True).strip();d=os.path.abspath('.');srcs=['src/cow/cow_utils.c','src/cow/cowdict.c','src/cow/cowset.c','src/cow/cowlist.c','src/cow/_cowmodule.c'];open('compile_commands.json','w').write(json.dumps([{'directory':d,'command':'cc -isysroot '+sdk+' -I'+inc+' -Isrc/cow -O2 -Wall -c '+s,'file':s}for s in srcs],indent=2))"

clean:
	rm -rf build/ src/*.egg-info src/**/*.egg-info
	find . -name '*.so' -delete
	find . -name '__pycache__' -exec rm -rf {} +

rebuild: clean build
