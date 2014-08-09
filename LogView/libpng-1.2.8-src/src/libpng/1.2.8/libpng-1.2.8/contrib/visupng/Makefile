RESDIR = ../../res
SRCDIR = ../../../libpng-1.2.8-src/contrib/visupng
VPATH = $(SRCDIR) $(RESDIR)

PROG=VisualPng
OBJSX=PngFile.o
LIBS=-lpng -lgdi32 -lz -lcomdlg32
LDFLAGS=-mwindows -L../..

CC=gcc
LINK=$(CC)
CFLAGS= -O3 -mms-bitfields  -fno-exceptions -fomit-frame-pointer -march=i386 -mcpu=i686 \
	-D__GW32__ -D_LARGEFILE_SOURCE=1 -D_LARGEFILE64_SOURCE=1 -D_FILE_OFFSET_BITS=64	\
	 -idirafter h:/glibc/include \
	-DPNG_USE_DLL 
LDFLAGS+=-s
C=.c
E=.exe
O=.o
OBJS=$(PROG)$(O) $(PROG)rc$(O) $(OBJSX)
OBJSXSRC=$(OBJSX:$(O)=$(C))

all: $(PROG)$(E)

.SUFFIXES:
.SUFFIXES: .c .o .rc .exe

.c.o: 
	$(CC) -c $(CFLAGS) $< -o $@

$(PROG)rc$(O): $(PROG).rc
	windres --include-dir=$(SRCDIR) --include-dir=$(RESDIR) -i $< -o $@

$(PROG)$(E): $(OBJS)
	$(LINK) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f *$(E) *$(O)
