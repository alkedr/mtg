RELEASE := y
BUILD_DIR := build

CXX := clang++ -std=c++11
MOC = moc
PACK = upx --best --ultra-brute -qq


ifeq ($(RELEASE), y)
	magic_BUILD_TYPE_FLAGS := -w -s -flto -fno-rtti
	server_BUILD_TYPE_FLAGS := -O3
	client_BUILD_TYPE_FLAGS := -Oz
	test_BUILD_TYPE_FLAGS := -O0
else
	magic_BUILD_TYPE_FLAGS := -Weverything -Wno-padded -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c99-extensions -Wno-weak-vtables -Wno-global-constructors -Wno-exit-time-destructors -Wno-undefined-reinterpret-cast -O0
	server_BUILD_TYPE_FLAGS :=
	client_BUILD_TYPE_FLAGS :=
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

client_HEADERS := $(magic_HEADERS) $(BUILD_DIR)/moc_client.cpp
client_SOURCES := $(magic_SOURCES) client.cpp
client_OBJECTS := $(BUILD_DIR)/client.o $(magic_OBJECTS)
client_FLAGS := $(magic_FLAGS) $(client_BUILD_TYPE_FLAGS) -fPIE $(shell pkg-config --cflags Qt5Gui Qt5Widgets | sed 's/-I\//-isystem\ \//g') -iquote build
client_LIBS  := $(magic_LIBS) $(shell pkg-config --libs Qt5Gui Qt5Widgets)

test_HEADERS := $(magic_HEADERS)
test_SOURCES := $(magic_SOURCES) test.cpp
test_OBJECTS := $(BUILD_DIR)/test.o $(magic_OBJECTS)
test_FLAGS := $(magic_FLAGS) $(test_BUILD_TYPE_FLAGS)

.PHONY : all clean

all: $(BUILD_DIR)/mtg-server $(BUILD_DIR)/mtg-client $(BUILD_DIR)/mtg-test


$(BUILD_DIR): Makefile
	@mkdir -p $@

$(BUILD_DIR)/moc_%.cpp: %.cpp Makefile
	@echo "MOC     $<"
	@$(MOC) -i $< -o $@

.SECONDEXPANSION:

$(BUILD_DIR)/%.o: %.cpp Makefile
	@echo "COMPILE $@"
	@$(CXX) -c $($*_FLAGS) $< -o $@

$(BUILD_DIR)/mtg-%: $$($$*_OBJECTS) $$($$*_HEADERS) Makefile
	@echo "LINK    $@"
	@$(CXX) $($*_FLAGS) $($*_LIBS) $($*_OBJECTS) -o $@

clean:
	rm -Rf $(BUILD_DIR)

