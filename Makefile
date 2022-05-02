TARGET_SO = libmoss.so
TARGET_AR = libmoss.a
OBJS =
DEPS = $(OBJS:.o=.d)

CPPFLAGS = -MMD -Iinclude
CFLAGS = -std=c11 -pedantic -pedantic-errors -O3 -Wall -Wextra
LDFLAGS = -shared
LDLIBS =

all: $(TARGET_SO) $(TARGET_AR)

$(TARGET_SO): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(TARGET_SO)

$(TARGET_AR): $(OBJS)
	$(AR) rcs $(TARGET_AR) $(OBJS)

clean:
	rm -rf $(TARGET_SO) $(TARGET_AR) $(OBJS) $(DEPS)

-include $(DEPS)
