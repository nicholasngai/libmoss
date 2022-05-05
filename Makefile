TARGET_SO = libmoss.so
TARGET_AR = libmoss.a
OBJS = \
	hashing.o \
	moss.o \
	winnowing.o \
	internal/multimap.o
DEPS = $(OBJS:.o=.d)

CPPFLAGS = -MMD -Iinclude
CFLAGS = -std=c11 -pedantic -pedantic-errors -O3 -Wall -Wextra
LDFLAGS = -shared
LDLIBS =

all: $(TARGET_SO) $(TARGET_AR) test

$(TARGET_SO): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(TARGET_SO)

$(TARGET_AR): $(OBJS)
	$(AR) rcs $(TARGET_AR) $(OBJS)

clean:
	rm -rf $(TARGET_SO) $(TARGET_AR) $(OBJS) $(DEPS)
	$(MAKE) -C test clean

test: $(TARGET_AR) FORCE
	$(MAKE) -C test

FORCE:

-include $(DEPS)
