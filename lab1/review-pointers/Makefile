BUILD  := build
STATIC := static

SOBJS :=	$(STATIC)/part1_harness.o \
		$(STATIC)/part2_harness.o \
		$(STATIC)/part3_harness.o \
		$(STATIC)/part4_harness.o \
		$(STATIC)/part5_harness.o

CC     := gcc
CFLAGS := -m32

OBJS :=	$(BUILD)/ex1 \
	$(BUILD)/ex2 \
	$(BUILD)/ex3 \
	$(BUILD)/ex4


all: $(OBJS)
	@:

$(BUILD)/ex%: ex%.c $(SOBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -g -c ex$*.c -o $(BUILD)/ex$*.o
	$(CC) $(CFLAGS) $(STATIC)/part$*_harness.o $(BUILD)/ex$*.o -lm -o $(BUILD)/ex$*

clean-logs: always
	rm -f *.out

clean: always clean-logs
	rm -rf $(BUILD)

.PHONY: all always

tidy: always
	git clean -dff
