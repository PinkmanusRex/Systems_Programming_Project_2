#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <dirent.h>

#include "jsd.h"
#include "tokenize.h"
#include "stringbuf.h"
#include "wf_table.h"
#include "helperR.h"
#include "collection_threads.h"
#include "debugger.h"
#include "sync_queue.h"
#include "extern_module.h"
#include "wf_repo.h"

int main(int argc, char *argv[]) {
        int fd = open(argv[1], O_RDONLY);
        wf_table *table = hash_create_table(argv[1], 30, 3.0);
        stringbuf *list = sb_create(10);
        tokenize(fd, table, list);
}
