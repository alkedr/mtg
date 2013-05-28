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

common_FLAGS := -pipe $(common_BUILD_TYPE_FLAGS) -pthread
comman_LIBS  :=

server_FLAGS := $(common_FLAGS) $(server_BUILD_TYPE_FLAGS)
server_LIBS  := $(comman_LIBS) -lboost_system

client_FLAGS := $(common_FLAGS) $(client_BUILD_TYPE_FLAGS) -fPIE $(shell pkg-config --cflags Qt5Gui Qt5Widgets | sed 's/-I\//-isystem\ \//g')
client_LIBS  := $(shell pkg-config --libs Qt5Gui Qt5Widgets)

test_FLAGS := $(common_FLAGS) $(test_BUILD_TYPE_FLAGS)

.PHONY : all clean

all: server client

#test

precompiled-server.hpp: precompiled.hpp
precompiled-client.hpp: precompiled.hpp

precompiled-server.hpp.pch: precompiled-server.hpp Makefile
	@echo PRECOMPILE $@
	@$(CXX) $(server_FLAGS) -x c++-header $< -o $@

server: server.cpp magic.hpp precompiled-server.hpp.pch Makefile
	@echo BUILD $@
	@$(CXX) $($@_FLAGS) $($@_LIBS) -include precompiled-server.hpp $< -o $@

precompiled-client.hpp.pch: precompiled-client.hpp Makefile
	@echo PRECOMPILE $@
	@$(CXX) $(client_FLAGS) -x c++-header $< -o $@

moc_client.cpp: client.cpp Makefile
	@echo MOC $@
	@$(MOC) $< -o $@

client: client.cpp moc_client.cpp magic.hpp precompiled-client.hpp.pch Makefile
	@echo BUILD $@
	@$(CXX) $($@_FLAGS) $($@_LIBS) -include precompiled-client.hpp $< -o $@
ifeq ($(RELEASE), y)
#	@strip --remove-section=.comment --remove-section=.note $@
	@upx --best --ultra-brute -qq $@
	@wc -c $@
endif

precompiled-test.hpp.pch: precompiled-test.hpp Makefile
	@echo PRECOMPILE $@
	@$(CXX) $(test_FLAGS) -x c++-header $< -o $@

test: test.cpp magic.hpp precompiled-test.hpp.pch Makefile
	@echo BUILD $@
	@$(CXX) $($@_FLAGS) $($@_LIBS) -include precompiled-client.hpp $< -o $@


clean:
	rm -Rf server client test moc_client.cpp 
