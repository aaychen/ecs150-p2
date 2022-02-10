# Target library
lib := libuthread.a
objs := queue.o uthread.o context.o preempt.o

# Don't print the commands unless explicitly requested with `make V=1`
ifneq ($(V),1)
Q = @
V = 0
endif

# gcc options
CC := gcc
CFLAGS := -Wall -Wextra -Werror -MMD

# ar options
AR := ar
ARFLAGS := -rcs

# Default rule
all: $(lib)

# Include dependencies
deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

# Rule for libuthread.a
$(lib): $(objs)
	@echo "AR $@"
	$(Q)$(AR) $(ARFLAGS) $@ $^

# Generic rule for compiling objects
%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

# Cleaning rule
clean:
	@echo "clean"
	$(Q)rm -f $(lib) $(objs) $(deps)
