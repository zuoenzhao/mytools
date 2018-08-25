CROSS	= arm-linux-

CC	= @echo " GCC	$@"; $(CROSS)gcc
CPP	= @echo " G++	$@"; $(CROSS)g++
LD	= @echo " LD	$@"; $(CROSS)ld
AR	= @echo " AR	$@"; $(CROSS)ar
RM	= @echo " RM	$@"; rm -f


CFLAGS	+= -Wall -g
CFLAGS	+= -DIMP_DBG
CFLAGS	+= -fomit-frame-pointer
CFLAGS	+= -Wstrict-prototypes

LDFLAGS	+=  -L./libs 

AFLAGS	+= -rv



TARGET	= ./$(Board)
	 


OBJS	= ./$(Board).o  



all: $(TARGET)
	@echo ""
	@echo "make success ----------------------------------"
	@echo ""
	cp $(TARGET) /home/share/zhaozuoen/ -rf


$(TARGET): $(OBJS) 
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC)  $(CFLAGS) -c -o  $@ $< 

.cpp.o:
	$(CPP) $(CFLAGS) -c -o $@ $<
clean:
	rm -f $(TARGET) $(OBJS);
	find ./ -type f \( -name '*.elf*' -o -name '*.gdb' \) | xargs rm -f

