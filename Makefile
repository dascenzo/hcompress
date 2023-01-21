BUILD ?= debug
ifneq ($(BUILD),debug)
  ifneq ($(BUILD),release)
    $(error Unknown build type: $(BUILD))
  endif
endif

build_dir = build-$(BUILD)
program = $(build_dir)/hcompress
sources = $(wildcard *.cpp)
objects = $(addprefix $(build_dir)/,$(sources:cpp=o))
dependencies = $(addprefix $(build_dir)/,$(sources:.cpp=.d))

CPPFLAGS = -MMD -MP
CXXFLAGS = -std=c++2a -Wall -Wextra
ifeq ($(BUILD),debug)
  CXXFLAGS += -O0 -g
else
  CXXFLAGS += -O3 -DNDEBUG
endif

.PHONY : all
all : $(program)

.PHONY : clean
clean :
	rm -rf $(build_dir)

$(program) : $(objects)
	$(CXX) $^ -o $@

$(objects) : | $(build_dir)

$(build_dir) :
	mkdir $(build_dir)

$(build_dir)/%.o : %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

-include $(dependencies)
