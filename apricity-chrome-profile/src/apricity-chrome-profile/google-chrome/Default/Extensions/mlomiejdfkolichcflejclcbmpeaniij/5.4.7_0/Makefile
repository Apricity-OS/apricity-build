SHELL=/bin/bash


GLUE=glue --optipng --css=data/css/build --img=data/images/build
JSHINT=./node_modules/.bin/jshint
MOCHA_PHANTOM=./node_modules/.bin/mocha-phantomjs
UGLIFYJS=./node_modules/.bin/uglifyjs
REQUIREJS_OPTIMIZER=./node_modules/.bin/r.js

INJECTED_SCRIPT_SOURCE=data/includes/ghostery.js
INJECTED_SCRIPT_TARGET=data/includes/ghostery-min.js


.PHONY: all buildjs buildsprites jshint json install test

all: $(INJECTED_SCRIPT_TARGET) jshint $(patsubst data/templates/%.html, data/templates/precompiled/%.js, $(wildcard data/templates/*.html)) buildjs

# TODO make this build only when necessary
buildjs:
	$(REQUIREJS_OPTIMIZER) -o rjs_build_config.js

buildsprites:
	$(GLUE) data/images/page_controls
	$(GLUE) data/images/panel/footer
	$(GLUE) data/images/panel/header
	$(GLUE) data/images/panel/settings
	$(GLUE) data/images/panel/tracker
	$(GLUE) data/images/panel/tutorial/tutorial_sprites

$(INJECTED_SCRIPT_TARGET): $(INJECTED_SCRIPT_SOURCE)
	$(JSHINT) $(INJECTED_SCRIPT_SOURCE)
	$(UGLIFYJS) $(INJECTED_SCRIPT_SOURCE) -cm --comments "/ \* /" -o $(INJECTED_SCRIPT_TARGET)
	@#cp $(INJECTED_SCRIPT_SOURCE) $(INJECTED_SCRIPT_TARGET)

jshint:
	@$(JSHINT) .

json:
	yaml2json --sort _locales/en/messages.{yml,json} && json2yaml --sort _locales/en/messages.{json,yml}

# TODO from http://stackoverflow.com/questions/9787160/makefile-that-compiles-all-cpp-files-in-a-directory-into-separate-executable
data/templates/precompiled/%.js: data/templates/%.html
	node tools/compile_template.js $@

test:
	@$(MOCHA_PHANTOM) testRunner.html

install:
	@npm install
