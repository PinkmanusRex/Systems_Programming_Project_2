CC = gcc
CFLAGS = -g -std=c99 -Wvla -Wall -pthread -fsanitize=address,undefined

all: compare

compare: compare.o collection_threads.o debugger.o helperR.o jsd.o stringbuf.o sync_queue.o tokenize.o wf_repo.o wf_table.o analysis_threads.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

compare.o: compare.c jsd.h tokenize.h stringbuf.h wf_table.h helperR.h collection_threads.h debugger.h sync_queue.h extern_module.h wf_repo.h analysis_threads.h
	$(CC) $(CFLAGS) -c $<

collection_threads.o: collection_threads.c collection_threads.h extern_module.h helperR.h stringbuf.h wf_repo.h wf_table.h sync_queue.h tokenize.h debugger.h
	$(CC) $(CFLAGS) -c $<

debugger.o: debugger.c debugger.h wf_repo.h wf_table.h
	$(CC) $(CFLAGS) -c $<

helperR.o: helperR.c helperR.h debugger.h extern_module.h
	$(CC) $(CFLAGS) -c $<

jsd.o: jsd.c jsd.h debugger.h
	$(CC) $(CFLAGS) -c $<

stringbuf.o: stringbuf.c stringbuf.h debugger.h
	$(CC) $(CFLAGS) -c $<

sync_queue.o: sync_queue.c sync_queue.h debugger.h
	$(CC) $(CFLAGS) -c $<

tokenize.o: tokenize.c tokenize.h stringbuf.h wf_table.h debugger.h
	$(CC) $(CFLAGS) -c $<

wf_repo.o: wf_repo.c wf_repo.h debugger.h
	$(CC) $(CFLAGS) -c $<

wf_table.o: wf_table.c wf_table.h debugger.h
	$(CC) $(CFLAGS) -c $<

analysis_threads.o: analysis_threads.c analysis_threads.h wf_table.h debugger.h extern_module.h
	$(CC) $(CFLAGS) -c $<

dcompare: dcompare.o dcollection_threads.o ddebugger.o dhelperR.o djsd.o dstringbuf.o dsync_queue.o dtokenize.o dwf_repo.o dwf_table.o danalysis_threads.o
	$(CC) $(CFLAGS) -o dcompare $^ -lm

dcompare.o: compare.c jsd.h tokenize.h stringbuf.h wf_table.h helperR.h collection_threads.h debugger.h sync_queue.h extern_module.h wf_repo.h analysis_threads.h
	$(CC) -c $(CFLAGS) -DDEBUG -o $@ $<

dcollection_threads.o: collection_threads.c collection_threads.h extern_module.h helperR.h stringbuf.h wf_repo.h wf_table.h sync_queue.h tokenize.h debugger.h
	$(CC) -c $(CFLAGS) -DDEBUG -o $@ $<

ddebugger.o: debugger.c debugger.h wf_repo.h wf_table.h
	$(CC) -c $(CFLAGS) -DDEBUG -o $@ $<

dhelperR.o: helperR.c helperR.h debugger.h extern_module.h
	$(CC) -c $(CFLAGS) -DDEBUG -o $@ $<

djsd.o: jsd.c jsd.h debugger.h
	$(CC) -c $(CFLAGS) -DDEBUG -o $@ $<

dstringbuf.o: stringbuf.c stringbuf.h debugger.h
	$(CC) -c $(CFLAGS) -DDEBUG -o $@ $<

dsync_queue.o: sync_queue.c sync_queue.h debugger.h
	$(CC) -c $(CFLAGS) -DDEBUG -o $@ $<

dtokenize.o: tokenize.c tokenize.h stringbuf.h wf_table.h debugger.h
	$(CC) -c $(CFLAGS) -DDEBUG -o $@ $<

dwf_repo.o: wf_repo.c wf_repo.h debugger.h
	$(CC) -c $(CFLAGS) -DDEBUG -o $@ $<

dwf_table.o: wf_table.c wf_table.h debugger.h
	$(CC) -c $(CFLAGS) -DDEBUG -o $@ $<

danalysis_threads.o: analysis_threads.c analysis_threads.h wf_table.h debugger.h extern_module.h
	$(CC) -c $(CFLAGS) -DDEBUG -o $@ $<

clean:
	rm -f *.o compare dcompare
