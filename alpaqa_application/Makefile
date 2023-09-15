CC?=$(CROSS_COMPILE)gcc
CFLAGS?=-I.
TARGET?=alpaqa_app
# -lm for math library
LDFLAGS?=-lm
OBJS?=$(TARGET).o PMSA003I.o SHT41.o alpaqaCalc.o
INCLUDES?=*.h

default all: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(INCLUDES) $(LDFLAGS)
clean:
	rm -f *.o $(TARGET)
