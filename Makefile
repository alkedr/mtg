RELEASE := n
BUILD_DIR := build

CXX := clang++ -std=c++11
MOC = moc
PACK = upx --best --ultra-brute -qq

SHELL := bash


ifeq ($(RELEASE), y)
	magic_BUILD_TYPE_FLAGS := -w -s -flto -fno-rtti
	server_BUILD_TYPE_FLAGS := -O3
	client_BUILD_TYPE_FLAGS := -Oz
	ai_BUILD_TYPE_FLAGS := -O3
	test_BUILD_TYPE_FLAGS := -O0
else
	magic_BUILD_TYPE_FLAGS := -O0 -ggdb3 -Weverything -Wno-unused-parameter -Wno-unused-macros -Wno-unused-member-function -Wno-padded -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c99-extensions -Wno-weak-vtables -Wno-global-constructors -Wno-exit-time-destructors -Wno-undefined-reinterpret-cast -Wno-c++1y-extensions
	server_BUILD_TYPE_FLAGS :=
	client_BUILD_TYPE_FLAGS :=
	ai_BUILD_TYPE_FLAGS :=
	test_BUILD_TYPE_FLAGS :=
endif

magic_HEADERS := magic.hpp
magic_SOURCES := magic.cpp
magic_OBJECTS := $(BUILD_DIR)/magic.o
magic_FLAGS := -pipe $(magic_BUILD_TYPE_FLAGS)

server_HEADERS := $(magic_HEADERS)
server_SOURCES := $(magic_SOURCES) server.cpp
server_OBJECTS := $(BUILD_DIR)/server.o $(magic_OBJECTS)
server_FLAGS := $(magic_FLAGS) $(server_BUILD_TYPE_FLAGS)
server_LIBS  := $(magic_LIBS) -lboost_system

client_HEADERS := $(magic_HEADERS) moc_client.cpp
client_SOURCES := $(magic_SOURCES) client.cpp
client_OBJECTS := $(BUILD_DIR)/client.o $(magic_OBJECTS)
client_FLAGS := $(magic_FLAGS) $(client_BUILD_TYPE_FLAGS) -fPIE $(shell pkg-config --cflags Qt5Gui Qt5Widgets | sed 's/-I\//-isystem\ \//g') -iquote build
client_LIBS  := $(magic_LIBS) $(shell pkg-config --libs Qt5Gui Qt5Widgets)

ai_HEADERS := $(magic_HEADERS)
ai_SOURCES := $(magic_SOURCES) ai.cpp
ai_OBJECTS := $(BUILD_DIR)/ai.o $(magic_OBJECTS)
ai_FLAGS := $(magic_FLAGS) $(ai_BUILD_TYPE_FLAGS)
ai_LIBS  := $(magic_LIBS)

test_HEADERS := $(magic_HEADERS)
test_SOURCES := $(magic_SOURCES) test.cpp
test_OBJECTS := $(BUILD_DIR)/test.o $(magic_OBJECTS)
test_FLAGS := $(magic_FLAGS) $(test_BUILD_TYPE_FLAGS) -frtti
test_LIBS := -lgtest

.PHONY : all clean

all: $(BUILD_DIR) $(BUILD_DIR)/server $(BUILD_DIR)/ai $(BUILD_DIR)/client $(BUILD_DIR)/unit_tests

$(BUILD_DIR): Makefile
	@mkdir -p $@


$(BUILD_DIR)/magic.o: magic.cpp magic.hpp Makefile
	@echo "COMPILE $@"
	@$(CXX) -c $(magic_FLAGS) $< -o $@

$(BUILD_DIR)/server.o: server.cpp magic.hpp Makefile
	@echo "COMPILE $@"
	@$(CXX) -c $(server_FLAGS) $< -o $@

$(BUILD_DIR)/client.o: client.cpp magic.hpp Makefile
	@echo "COMPILE $@"
	@cat <(cat $<) <(moc $<) | $(CXX) -c -x c++ $(client_FLAGS) -o $@ -

$(BUILD_DIR)/ai.o: ai.cpp magic.hpp Makefile
	@echo "COMPILE $@"
	@$(CXX) -c $(ai_FLAGS) $< -o $@

$(BUILD_DIR)/test.o: test.cpp magic.hpp Makefile
	@echo "COMPILE $@"
	@$(CXX) -c $(test_FLAGS) $< -o $@


$(BUILD_DIR)/server: $(server_OBJECTS) Makefile
	@echo "LINK    $@"
	@$(CXX) $(server_FLAGS) $(server_LIBS) $(server_OBJECTS) -o $@

$(BUILD_DIR)/client: $(client_OBJECTS) Makefile
	@echo "LINK    $@"
	@$(CXX) $(client_FLAGS) $(client_LIBS) $(client_OBJECTS) -o $@
ifeq ($(RELEASE), y)
	@echo "PACK    $@"
	@$(PACK) $@
endif

$(BUILD_DIR)/ai: $(ai_OBJECTS) Makefile
	@echo "LINK    $@"
	@$(CXX) $(ai_FLAGS) $(ai_LIBS) $(ai_OBJECTS) -o $@

$(BUILD_DIR)/unit_tests: $(test_OBJECTS) Makefile
	@echo "LINK    $@"
	@$(CXX) $(test_FLAGS) $(test_LIBS) $(test_OBJECTS) -o $@


clean:
	rm -Rf $(BUILD_DIR)

