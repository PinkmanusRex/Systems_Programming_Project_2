CC = gcc
CFLAGS = -g -std=c99 -Wvla -Wall -pthread -fsanitize=address,undefined

all: compare

compare: compare.o collection_threads.o debugger.o helperR.o jsd.o stringbuf.o sync_queue.o tokenize.o wf_repo.o wf_table.o
	$(CC) $(CFLAGS) -o $@ $^

compare.o: compare.c jsd.h tokenize.h stringbuf.h wf_table.h helperR.h collection_threads.h debugger.h sync_queue.h extern_module.h wf_repo.h
	$(CC) $(CFLAGS) -c $<

collection_threads.o: collection_threads.c collection_threads.h extern_module.h helperR.h stringbuf.h wf_repo.h wf_table.h sync_queue.h tokenize.h
	$(CC) $(CFLAGS) -c $<

debugger.o: debugger.c debugger.h wf_repo.h wf_table.h
	$(CC) $(CFLAGS) -c $<

helperR.o: helperR.c helperR.h
	$(CC) $(CFLAGS) -c $<

jsd.o: jsd.c jsd.h
	$(CC) $(CFLAGS) -c $<

stringbuf.o: stringbuf.c stringbuf.h
	$(CC) $(CFLAGS) -c $<

sync_queue.o: sync_queue.c sync_queue.h
	$(CC) $(CFLAGS) -c $<

tokenize.o: tokenize.c tokenize.h stringbuf.h wf_table.h
	$(CC) $(CFLAGS) -c $<

wf_repo.o: wf_repo.c wf_repo.h
	$(CC) $(CFLAGS) -c $<

wf_table.o: wf_table.c wf_table.h
	$(CC) $(CFLAGS) -c $<
