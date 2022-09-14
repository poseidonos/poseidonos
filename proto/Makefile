TOP := $(abspath $(CURDIR)/..)
SRCS += $(wildcard generated/cpp/*.cc)

OBJECTS = $(SRCS:.cc=.o)


all : $(OBJECTS)

%.o: %.cpp
	@echo "Compiling [$(notdir $<)]..."
	@$(CC) $(INCLUDE) $(CPPFLAGS) -MMD -MP $(DEFINE) -c $< -o $@

.PHONY:clean
clean:
	@rm -rf generated/cpp/*.o

-include $(OBJECTS:.o=.d)

