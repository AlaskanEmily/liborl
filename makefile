OBJECTS=orlentry.o orlflhnd.o orlhash.o memicmp.o strcasecmp.o strcasestr.o
COFFOBJECTS=coffentr.o coffflhn.o coffimpl.o coffload.o cofflwlv.o
ELFOBJECTS=elfentr.o elfflhn.o elfload.o elflwlv.o

all: liborl.so liborl-static.a

AR?=ar
RANLIB?=ranlib

CC?=cc
LINK?=cc

CCFLAGS?=-Wall -Werror -pedantic -g 
CFLAGS?=$(CCFLAGS)
CXXFLAGS?=$(CCFLAGS) -std=c++98 -fno-rtti -fno-exceptions

AR?=ar
RANLIB?=ranlib

ORL_FEATURE_FLAGS?=-DORL_ENABLE_ELF -DORL_ENABLE_COFF
CIFLAGS=$(CFLAGS) $(ORL_FEATURE_FLAGS) -Iinclude 
ORLCIFLAGS=$(CIFLAGS) -ansi
COFFCIFLAGS=$(CIFLAGS) -Iinclude/coff -ansi
OMFCIFLAGS=$(CIFLAGS) -Iinclude/omf -ansi
ELFCIFLAGS=$(CIFLAGS) -Iinclude/elf -std=c99

CXXIFLAGS=$(CXXFLAGS) -Iinclude

# ORL core
orlentry.o: src/orlentry.c include/orl.h include/orllevel.h include/orlentry.h include/orlflhnd.h include/omf/pcobj.h
	$(CC) $(ORLCIFLAGS) -c src/orlentry.c -o orlentry.o

orlflhnd.o: src/orlflhnd.c include/orllevel.h include/orlflhnd.h
	$(CC) $(ORLCIFLAGS) -c src/orlflhnd.c -o orlflhnd.o

orlhash.o: src/orlhash.c include/orlhash.h include/orllevel.h include/orlhshdf.h
	$(CC) $(ORLCIFLAGS) -c src/orlhash.c -o orlhash.o

memicmp.o: src/memicmp.c src/memicmp.h
	$(CC) $(CFLAGS) -c src/memicmp.c -o memicmp.o

strcasecmp.o: src/strcasecmp.c src/strcasecmp.h src/str-two-way.h
	$(CC) $(CFLAGS) -c src/strcasecmp.c -o strcasecmp.o

strcasestr.o: src/strcasestr.c src/strcasestr.h src/str-two-way.h
	$(CC) $(CFLAGS) -c src/strcasestr.c -o strcasestr.o

liborl.so: $(OBJECTS) $(COFFOBJECTS) $(ELFOBJECTS)
	$(LINK) -g $(COFFOBJECTS) $(ELFOBJECTS) $(OBJECTS) -shared -o liborl.so

liborl-static.a: $(OBJECTS) $(COFFOBJECTS) $(ELFOBJECTS)
	$(AR) -rc liborl-static.a $(OBJECTS)
	$(RANLIB) liborl-static.a

# COFF library

# TODO
coffentr.o: src/coff/coffentr.c
	$(CC) $(COFFCIFLAGS) -c src/coff/coffentr.c -o coffentr.o

# TODO
coffflhn.o: src/coff/coffflhn.c
	$(CC) $(COFFCIFLAGS) -c src/coff/coffflhn.c -o coffflhn.o

# TODO
coffimpl.o: src/coff/coffimpl.c
	$(CC) $(COFFCIFLAGS) -c src/coff/coffimpl.c -o coffimpl.o

# TODO
coffload.o: src/coff/coffload.c
	$(CC) $(COFFCIFLAGS) -c src/coff/coffload.c -o coffload.o

# TODO
cofflwlv.o: src/coff/cofflwlv.c
	$(CC) $(COFFCIFLAGS) -c src/coff/cofflwlv.c -o cofflwlv.o

# ELF library

# TODO
elfentr.o: src/elf/elfentr.c
	$(CC) $(ELFCIFLAGS) -c src/elf/elfentr.c -o elfentr.o

# TODO
elfflhn.o: src/elf/elfflhn.c
	$(CC) $(ELFCIFLAGS) -c src/elf/elfflhn.c -o elfflhn.o

# TODO
elfload.o: src/elf/elfload.c
	$(CC) $(ELFCIFLAGS) -c src/elf/elfload.c -o elfload.o

# TODO
elflwlv.o: src/elf/elflwlv.c
	$(CC) $(ELFCIFLAGS) -c src/elf/elflwlv.c -o elflwlv.o

clean:
	rm *.so *.o

#test: test.c liborl.so
#	$(CC) $(CIFLAGS) -Wl,-Map,test.map test.c $(COFFOBJECTS) $(ELFOBJECTS) $(OBJECTS) -o test
