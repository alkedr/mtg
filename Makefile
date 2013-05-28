RELEASE := n

CXX := clang++ -std=c++11
MOC = moc
PACK = upx --best --ultra-brute -qq


ifeq ($(RELEASE), y)
	common_BUILD_TYPE_FLAGS := -w -s -flto -fno-rtti
	server_BUILD_TYPE_FLAGS := -O3
	client_BUILD_TYPE_FLAGS := -Oz
	test_BUILD_TYPE_FLAGS := -O0
else
	common_BUILD_TYPE_FLAGS := -Weverything -Wno-padded -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c99-extensions -Wno-weak-vtables -Wno-global-constructors -Wno-exit-time-destructors -Wno-undefined-reinterpret-cast -O0
	server_BUILD_TYPE_FLAGS :=
	client_BUILD_TYPE_FLAGS :=
	test_BUILD_TYPE_FLAGS :=
endif

common_HEADERS := magic.hpp precompiled.hpp
common_FLAGS := -pipe $(common_BUILD_TYPE_FLAGS) -pthread
comman_LIBS  :=

server_HEADERS := $(common_HEADERS) precompiled-server.hpp
server_SOURCES := $(common_SOURCES) server.cpp
server_FLAGS := $(common_FLAGS) $(server_BUILD_TYPE_FLAGS)
server_LIBS  := $(comman_LIBS) -lboost_system

client_HEADERS := $(common_HEADERS) precompiled-client.hpp moc_client.cpp
client_SOURCES := $(common_SOURCES) client.cpp
client_FLAGS := $(common_FLAGS) $(client_BUILD_TYPE_FLAGS) -fPIE $(shell pkg-config --cflags Qt5Gui Qt5Widgets | sed 's/-I\//-isystem\ \//g')
client_LIBS  := $(common_LIBS) $(shell pkg-config --libs Qt5Gui Qt5Widgets)

test_HEADERS := precompiled-test.hpp
test_SOURCES := test.cpp
test_FLAGS := $(common_FLAGS) $(test_BUILD_TYPE_FLAGS)

.PHONY : all clean

all: mtg-server mtg-client

#test

.SECONDARY:

precompiled-%.hpp.pch: precompiled-%.hpp precompiled.hpp Makefile
	@echo PRECOMPILE $<
	@$(CXX) $($*_FLAGS) -x c++-header $< -o $@

moc_%.cpp: %.cpp Makefile
	@echo MOC $<
	@$(MOC) $< -o $@

.SECONDEXPANSION:
mtg-%: precompiled-%.hpp.pch $$($$*_SOURCES) $$($$*_HEADERS) Makefile
	@echo BUILD $@
	@$(CXX) $($*_FLAGS) $($*_LIBS) -include precompiled-$*.hpp $($*_SOURCES) -o $@

clean:
	rm -Rf mtg-server mtg-client mtg-test moc_client.cpp 

