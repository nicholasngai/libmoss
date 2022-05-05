TARGET_SO = libmoss.so
TARGET_AR = libmoss.a
OBJS = \
	hashing.o \
	moss.o \
	winnowing.o \
	internal/multimap.o
DEPS = $(OBJS:.o=.d)

TOOLS_TARGETS = \
	tools/moss
TOOLS_OBJS = $(TOOLS_TARGETS:=.o)
TOOLS_DEPS = $(TOOLS_OBJS:.o=.d)

TOKENIZER_TARGETS = \
	tools/moss_tokenizer_go

CPPFLAGS = -MMD -Iinclude
CFLAGS = -std=c11 -pedantic -pedantic-errors -O3 -Wall -Wextra
LDFLAGS = -shared
LDLIBS =

TOOLS_CPPFLAGS = -MMD -Iinclude
TOOLS_CFLAGS = -std=c11 -pedantic -pedantic-errors -O3 -Wall -Wextra
TOOLS_LDFLAGS = -L.
TOOLS_LDLIBS = -lmoss

all: $(TARGET_SO) $(TARGET_AR) \
	$(TOOLS_TARGETS) \
	$(TOKENIZER_TARGETS) \
	test

# libmoss library.

$(TARGET_SO): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(TARGET_SO)

$(TARGET_AR): $(OBJS)
	$(AR) rcs $(TARGET_AR) $(OBJS)

# libmoss tools.

tools/%.o: CPPFLAGS = $(TOOLS_CPPFLAGS)
tools/%.o: CFLAGS = $(TOOLS_CFLAGS)
tools/%: LDFLAGS = $(TOOLS_LDFLAGS)
tools/%: LDLIBS = $(TOOLS_LDLIBS)
tools/%: tools/%.o $(TARGET_SO)
	$(CC) $(LDFLAGS) $< $(LDLIBS) -o $@

# Tokenizers.

tools/moss_tokenizer_go: tools/tokenizers/go/moss_tokenizer_go.go
	go build -o $@ $^

# Tester.

test: $(TARGET_AR) FORCE
	$(MAKE) -C test

# Misc.

clean:
	rm -rf \
		$(TARGET_SO) $(TARGET_AR) $(OBJS) $(DEPS) \
		$(TOOLS_TARGETS) $(TOOLS_OBJS) $(TOOLS_DEPS) \
		$(TOKENIZER_TARGETS)
	$(MAKE) -C test clean

FORCE:

-include $(DEPS) $(TOOLS_DEPS)
