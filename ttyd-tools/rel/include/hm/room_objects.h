#pragma once

#include <gc/OSLink.h>

void robjects_init();
void robjects_oslink_hook(gc::OSLink::OSModuleInfo *);