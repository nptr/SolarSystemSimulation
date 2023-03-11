EXECUTABLE ?= SolarSystemSimulation

BUILD_DIR ?= ./build
SRC_DIRS ?= ./SolarSystemSimulation++

#---------------------------------------------------------------------------------
# Compiler and Linker inputs
#---------------------------------------------------------------------------------
SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP
ASFLAGS ?=
CFLAGS ?=
CXXFLAGS ?=
LDFLAGS := -lm -lstdc++ -lGL -lglfw

#---------------------------------------------------------------------------------
# Build rules
#---------------------------------------------------------------------------------
$(BUILD_DIR)/$(EXECUTABLE): $(OBJS)
	@echo built ... $(notdir $@)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.s.o: %.s
	@mkdir -p $(dir $@)
	@echo $<
	@$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	@echo $<
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@echo $<
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

#---------------------------------------------------------------------------------
# Housekeeping
#---------------------------------------------------------------------------------
.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)