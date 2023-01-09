#pragma once

#include <gc/OSLink.h>

void br_init();
void br_oslink_hook(gc::OSLink::OSModuleInfo *);
const char* br_filealloc_hook(const char *);
const char* br_msgsearch_hook(const char *);
void br_patch();