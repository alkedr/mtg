RELEASE := n
BUILD_DIR := build

CXX := clang++ -std=c++11
MOC := moc
PACK := upx --best --ultra-brute -qq


# name_OBJECTS for each executable
server_OBJECTS := magic.o protocol.o server.o
gui_qt_OBJECTS := magic.o protocol.o client.o gui_qt.o
ai_OBJECTS     := magic.o protocol.o client.o ai.o
test_OBJECTS   := magic.o test.o

# name_CXXFLAGS for each .cpp file
gui_qt_CXXFLAGS := -fPIE $(shell pkg-config --cflags Qt5Gui Qt5Widgets | sed 's/-I\//-isystem\ \//g') -iquote build
test_CXXFLAGS   := -frtti

# name_LFLAGS for each executable
server_LFLAGS := -lboost_system -lprotobuf
gui_qt_LFLAGS := -lboost_system -lprotobuf $(shell pkg-config --libs Qt5Gui Qt5Widgets)
ai_LFLAGS     := -lboost_system -lprotobuf
test_LFLAGS   := -frtti -lgtest

# name_DEPS for each .cpp file
magic_DEPS     := magic.hpp
client_DEPS    := magic.hpp $(BUILD_DIR)/protocol.pb.h client.hpp
protocol_DEPS  := magic.hpp
ai_DEPS        := magic.hpp $(BUILD_DIR)/protocol.pb.h client.hpp
server_DEPS    := magic.hpp $(BUILD_DIR)/protocol.pb.h
test_DEPS      := magic.hpp
gui_qt_DEPS    := magic.hpp $(BUILD_DIR)/protocol.pb.h client.hpp $(BUILD_DIR)/moc_gui_qt.hpp

ifeq ($(RELEASE), y)
	FLAGS := -pipe -flto -Oz -g0 -fno-rtti
	CXXFLAGS := -w
	LFLAGS := -s
else
	FLAGS := -pipe -O0 -ggdb3 -fno-rtti
	CXXFLAGS := -Weverything -Wno-unused-parameter -Wno-unused-macros -Wno-unused-member-function -Wno-padded -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c99-extensions -Wno-weak-vtables -Wno-global-constructors -Wno-exit-time-destructors -Wno-undefined-reinterpret-cast -Wno-c++1y-extensions
	LFLAGS := 
endif


.PHONY: all clean

all: $(BUILD_DIR) $(BUILD_DIR)/mtg-server $(BUILD_DIR)/mtg-ai $(BUILD_DIR)/mtg-gui_qt $(BUILD_DIR)/mtg-test

$(BUILD_DIR): Makefile
	@mkdir -p $@

.SECONDEXPANSION:
.SECONDARY:

$(BUILD_DIR)/moc_%.hpp: %.cpp Makefile
	@echo "MOC     $@"
	@$(MOC) $< -o $@

$(BUILD_DIR)/%.o: %.cpp $$($$*_DEPS) Makefile
	@echo "COMPILE $@"
	@$(CXX) -c $(FLAGS) $(CXXFLAGS) $($*_CXXFLAGS) $< -o $@

$(BUILD_DIR)/mtg-%: $$(addprefix $(BUILD_DIR)/,$$($$*_OBJECTS))
	@echo "LINK    $@"
	@$(CXX) $(FLAGS) $(LFLAGS) $($*_LFLAGS) $^ -o $@
ifeq ($(RELEASE), y)
	@echo "PACK    $@"
	@$(PACK) $@
endif

$(BUILD_DIR)/protocol.o: $(BUILD_DIR)/protocol.pb.cc $(BUILD_DIR)/protocol.pb.h Makefile
	@echo "COMPILE $@"
	@$(CXX) -c $(FLAGS) $(CXXFLAGS) -w $< -o $@

$(BUILD_DIR)/protocol.pb.h $(BUILD_DIR)/protocol.pb.cc: protocol.proto Makefile
	@echo "PROTOC  $@"
	@protoc --cpp_out=build protocol.proto

clean:
	rm -Rf $(BUILD_DIR)


