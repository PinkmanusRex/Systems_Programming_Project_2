#ifndef DEBUGGER_H
#define DEBUGGER_H
#include "wf_repo.h"

#ifndef DEBUG
#define DEBUG 0
#else
#define DEBUG 1
#endif

int debug_wf_repo_print(wf_repo *);

#endif
