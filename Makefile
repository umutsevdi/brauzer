#BUILD | TEST | RELEASE
SRCDIR := src
INCDIR := include
OBJDIR := obj
BUILDDIR := bin

# Compiler and flags
CC := gcc
# -g for debug
CFLAGS := -Wall -Wextra -I$(INCDIR) -pthread

ifeq ($(TYPE), RELEASE)
    CFLAGS += -O2
else
	CFLAGS += -g
endif

# Source files (add more if necessary)
SRCS      := $(filter-out $(SRCDIR)/test.c, $(wildcard $(SRCDIR)/*.c))
SRCS_TEST := $(filter-out $(SRCDIR)/main.c, $(wildcard $(SRCDIR)/*.c))
# Object files derived from source files
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
OBJS_TEST := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS_TEST))

# Main build target
all: $(BUILDDIR)/build
test: $(BUILDDIR)/test

# Rule to build the executable from object files
$(BUILDDIR)/test: $(OBJS_TEST)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@
# Rule to build the executable from object files
$(BUILDDIR)/build: $(OBJS)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@
# Rule to build object files from source files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	echo step $@
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@
# -o $@
# Clean rule to remove object files and the output executable
clean:
	rm -rf $(OBJDIR) $(BUILDDIR)

# Phony targets to prevent conflicts with file names
.PHONY: all clean
