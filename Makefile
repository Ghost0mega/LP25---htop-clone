CC := gcc
CFLAGS := -std=c11 -Iinclude -Wall -Wextra -O2 -g
# libs go here 
LDFLAGS :=

SRCDIR := src
BUILDDIR := build
BINDIR := bin
TARGET := htop-clone

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(OBJS) | $(BINDIR)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR) $(BINDIR):
	mkdir -p $@

clean:
# 	delete build and bin directories
	rm -rf $(BUILDDIR) $(BINDIR)

run: all
	./$(BINDIR)/$(TARGET)
