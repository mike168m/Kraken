#gcc-8 -s kraken_test_x86.c -o kraken_test_x86 -ggdb -m64 -Wall

ifeq ($(shell uname), Darwin)
	APPLE_CCFLAGS = -m64
	APPLE_ASFLAGS = -arch x86_64
endif

CFLAGS = $(APPLE_CCFLAGS) -g -Wall

kraken_test_x86: kraken_test_x86.o
	$(CC) $(APPLE_CCFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f *.o kraken_test_x86