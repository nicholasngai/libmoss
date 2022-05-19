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

TEST_TARGET = test/test
TEST_OBJ = $(TEST_TARGET:=.o)
TEST_DEP = $(TEST_TARGET:=.d)

CPPFLAGS = -MMD -Iinclude
CFLAGS = -std=c11 -pedantic -pedantic-errors -O3 -Wall -Wextra
LDFLAGS = -shared
LDLIBS = \
	-lpthread

TOOLS_CPPFLAGS = -MMD -Iinclude
TOOLS_CFLAGS = -std=c11 -pedantic -pedantic-errors -O3 -Wall -Wextra
TOOLS_LDFLAGS = -L.
TOOLS_LDLIBS = -lmoss \
	-lpthread

TEST_CPPFLAGS = -MMD -Iinclude
TEST_CFLAGS = -Wall -Wextra -g
TEST_LDFLAGS =
TEST_LDLIBS = \
	-lpthread

all: $(TARGET_SO) $(TARGET_AR) \
	$(TOOLS_TARGETS) \
	$(TOKENIZER_TARGETS) \
	$(TEST_TARGET)

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

$(TEST_TARGET): CPPFLAGS = $(TEST_CPPFLAGS)
$(TEST_TARGET): CFLAGS = $(TEST_CFLAGS)
$(TEST_TARGET): LDFLAGS = $(TEST_LDFLAGS)
$(TEST_TARGET): LDLIBS = $(TEST_LDLIBS)
$(TEST_TARGET): $(TEST_OBJ) $(TARGET_AR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Misc.

clean:
	rm -rf \
		$(TARGET_SO) $(TARGET_AR) $(OBJS) $(DEPS) \
		$(TOOLS_TARGETS) $(TOOLS_OBJS) $(TOOLS_DEPS) \
		$(TOKENIZER_TARGETS) \
		$(TEST_TARGET) $(TEST_OBJ) $(TEST_DEP)

-include $(DEPS) $(TOOLS_DEPS) $(TEST_DEP)
