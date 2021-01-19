DEPS = ca0132_defs.h hda_hwdep.h
CFLAGS = -O2 -Wall

BASE_OBJS = ca0132_base_functions.o
DSP_OBJS  = ca0132_dsp_functions.o
targets = ca0132-8051-write-exram-from-file ca0132-chipio-read-data ca0132-8051-dump-state \
	ca0132-8051-read-exram ca0132-8051-read-exram-to-file ca0132-8051-write-exram \
	ca0132-8051-command-line ca0132-chipio-read-to-file ca0132-chipio-write-data \
	ca0132-chipio-write-data-from-file ca0132-dsp-assembler \
	ca0132-dsp-disassembler ca0132-dsp-op-test \
	ca0132-frame-dump-formatted ca0132-get-chipio-flags \
	ca0132-get-chipio-stream-data ca0132-get-chipio-stream-ports \
	ca0132-send-dsp-scp-cmd

.PHONY: clean all
all : $(targets)
clean:
	rm -f  $(targets) $(BASE_OBJS) $(DSP_OBJS)

ca0132-8051-write-exram-from-file: $(BASE_OBJS) ca0132-8051-write-exram-from-file.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132-chipio-read-data: $(BASE_OBJS) ca0132-chipio-read-data.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132-8051-dump-state: $(BASE_OBJS) ca0132-8051-dump-state.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132-8051-read-exram: $(BASE_OBJS) ca0132-8051-read-exram.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132-8051-read-exram-to-file: $(BASE_OBJS) ca0132-8051-read-exram-to-file.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132-8051-write-exram: $(BASE_OBJS) ca0132-8051-write-exram.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132-8051-command-line: $(BASE_OBJS) ca0132-8051-command-line.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132-chipio-read-to-file: $(BASE_OBJS) ca0132-chipio-read-to-file.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132-chipio-write-data: $(BASE_OBJS) ca0132-chipio-write-data.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132-chipio-write-data-from-file: $(BASE_OBJS) ca0132-chipio-write-data-from-file.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132-dsp-assembler: $(DSP_OBJS) ca0132-dsp-assembler.c
	gcc $@.c -o $@ $(DSP_OBJS) $(CFLAGS)

ca0132-dsp-disassembler: $(DSP_OBJS)  ca0132-dsp-disassembler.c
	gcc $@.c -o $@ $(DSP_OBJS) $(CFLAGS)

ca0132-dsp-op-test: $(BASE_OBJS) $(DSP_OBJS) ca0132-dsp-op-test.c
	gcc $@.c -o $@ $(DSP_OBJS) $(BASE_OBJS) $(CFLAGS)

ca0132-frame-dump-formatted: $(BASE_OBJS) ca0132-frame-dump-formatted.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132-get-chipio-flags: $(BASE_OBJS) ca0132-get-chipio-flags.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132-get-chipio-stream-data: $(BASE_OBJS) ca0132-get-chipio-stream-data.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132-get-chipio-stream-ports: $(BASE_OBJS) ca0132-get-chipio-stream-ports.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132-send-dsp-scp-cmd: $(BASE_OBJS) ca0132-send-dsp-scp-cmd.c
	gcc $@.c -o $@ $(BASE_OBJS) $(CFLAGS)

ca0132_dsp_functions.o: ca0132_dsp_functions.c $(DEPS)
	gcc -c $< $(CFLAGS)

ca0132_base_functions.o: ca0132_base_functions.c $(DEPS)
	gcc -c $< $(CFLAGS)
