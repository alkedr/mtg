RELEASE := y
BUILD_DIR := build

CXX := clang++ -std=c++11
MOC = moc
PACK = upx --best --ultra-brute -qq

SHELL := bash


ifeq ($(RELEASE), y)
	magic_BUILD_TYPE_FLAGS := -w -s -flto -fno-rtti
	server_BUILD_TYPE_FLAGS := -O3
	gui_qt_BUILD_TYPE_FLAGS := -Oz
	ai_BUILD_TYPE_FLAGS := -O3
	test_BUILD_TYPE_FLAGS := -O0
else
	magic_BUILD_TYPE_FLAGS := -O0 -ggdb3 -Weverything -Wno-unused-parameter -Wno-unused-macros -Wno-unused-member-function -Wno-padded -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c99-extensions -Wno-weak-vtables -Wno-global-constructors -Wno-exit-time-destructors -Wno-undefined-reinterpret-cast -Wno-c++1y-extensions
	server_BUILD_TYPE_FLAGS :=
	gui_qt_BUILD_TYPE_FLAGS :=
	ai_BUILD_TYPE_FLAGS :=
	test_BUILD_TYPE_FLAGS :=
endif

magic_HEADERS := magic.hpp
magic_SOURCES := magic.cpp
magic_FLAGS := -pipe $(magic_BUILD_TYPE_FLAGS)

client_HEADERS := client.hpp
client_SOURCES := client.cpp
client_FLAGS := 

server_HEADERS := magic.hpp
server_SOURCES := magic.cpp server.cpp
server_OBJECTS := $(BUILD_DIR)/server.o $(BUILD_DIR)/magic.o
server_FLAGS := $(magic_FLAGS) $(server_BUILD_TYPE_FLAGS)
server_LIBS  := $(magic_LIBS) -lboost_system

gui_qt_HEADERS := magic.hpp client.hpp moc_gui_qt.cpp
gui_qt_SOURCES := magic.cpp client.cpp gui_qt.cpp
gui_qt_OBJECTS := $(BUILD_DIR)/gui_qt.o $(BUILD_DIR)/magic.o $(BUILD_DIR)/client.o
gui_qt_FLAGS := $(magic_FLAGS) $(client_FLAGS) $(gui_qt_BUILD_TYPE_FLAGS) -fPIE $(shell pkg-config --cflags Qt5Gui Qt5Widgets | sed 's/-I\//-isystem\ \//g') -iquote build
gui_qt_LIBS  := $(magic_LIBS) $(client_LIBS) $(shell pkg-config --libs Qt5Gui Qt5Widgets)

ai_HEADERS := magic.hpp client.hpp
ai_SOURCES := magic.cpp client.cpp ai.cpp
ai_OBJECTS := $(BUILD_DIR)/ai.o $(BUILD_DIR)/magic.o $(BUILD_DIR)/client.o
ai_FLAGS := $(magic_FLAGS) $(client_FLAGS) $(ai_BUILD_TYPE_FLAGS)
ai_LIBS  := $(magic_LIBS) $(client_LIBS)

test_HEADERS := magic.hpp
test_SOURCES := magic.cpp test.cpp
test_OBJECTS := $(BUILD_DIR)/test.o $(BUILD_DIR)/magic.o
test_FLAGS := $(magic_FLAGS) $(test_BUILD_TYPE_FLAGS) -frtti
test_LIBS := -lgtest


.PHONY : all clean

all: $(BUILD_DIR) $(BUILD_DIR)/mtg-server $(BUILD_DIR)/mtg-ai $(BUILD_DIR)/mtg-gui_qt $(BUILD_DIR)/mtg-test

$(BUILD_DIR): Makefile
	@mkdir -p $@


define compile = 
	@echo "COMPILE $@"
	@$(CXX) -c $($*_FLAGS) $< -o $@
endef

define link = 
	@echo "LINK    $@"
	@$(CXX) $($*_FLAGS) $($*_LIBS) $($*_OBJECTS) -o $@
endef

define pack =
	@echo "PACK    $@"
	@$(PACK) $@
endef


.SECONDEXPANSION:
.SECONDARY:

$(BUILD_DIR)/%.o: %.cpp $($*_HEADERS) Makefile
	$(compile)

$(BUILD_DIR)/gui_qt.o: gui_qt.cpp magic.hpp Makefile
	@echo "COMPILE $@"
	@cat <(cat $<) <(moc $<) | $(CXX) -c -x c++ $(gui_qt_FLAGS) -o $@ -

$(BUILD_DIR)/mtg-%: $$($$*_OBJECTS) Makefile
	$(link)
ifeq ($(RELEASE), y)
	$(pack)
endif

clean:
	rm -Rf $(BUILD_DIR)

