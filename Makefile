CXX ?= g++
CXXFLAGS += -Wall -Wextra -O2 -pipe -fstack-protector-strong -std=c++17
LDFLAGS += -lbcm2835 -lpthread -lrt -lm

APP_VERSION := $(shell cat VERSION)
CXXFLAGS += -DAPP_VERSION=\"$(APP_VERSION)\"

ARCH := $(shell uname -m)
ifeq ($(ARCH),armv7l)
    CXXFLAGS += -march=armv7-a -mfloat-abi=hard -mfpu=neon
else ifeq ($(ARCH),aarch64)
    CXXFLAGS += -march=armv8-a -mtune=cortex-a72
endif

TARGET := bin/laser_measure

SRC_DIR := src
INC_DIR := include
OBJ_DIR := obj
BIN_DIR := bin

SRCS := $(wildcard $(SRC_DIR)/**/*.cpp $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

INCLUDES := $(addprefix -I,$(shell find $(INC_DIR) -type d))

.PHONY: all clean distclean install uninstall run

all: $(TARGET)

run: $(TARGET)
	@if [ "$(shell whoami)" != "root" ]; then \
		echo "Ejecutando con sudo..."; \
		sudo $(TARGET); \
	else \
		$(TARGET); \
	fi

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@ -MF $(@:.o=.d)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET)
	rm -f $(DEPS)

distclean: clean
	rm -rf $(BIN_DIR)

install: all
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(TARGET) $(DESTDIR)/usr/local/bin/laser_measure

uninstall:
	rm -f $(DESTDIR)/usr/local/bin/laser_measure

-include $(DEPS)
