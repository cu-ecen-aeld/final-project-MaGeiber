CC?=$(CROSS_COMPILE)gcc
CFLAGS?=-I.
TARGET?=alpaqa_app
#LDFLAGS?=-pthread -lrt
OBJS?=$(TARGET).o

default all: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(INCLUDES) $(LDFLAGS)
clean:
	rm -f *.o $(TARGET)
