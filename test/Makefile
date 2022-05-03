TARGET = test
OBJS = \
	test.o
DEPS = $(OBJS:.o=.d)

CPPFLAGS = -MMD
CFLAGS = -g -I../include
LDLIBS = -L.. -l:libmoss.a

all: $(TARGET)

$(TARGET): $(OBJS)
