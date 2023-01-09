#pragma once

#include <gc/OSLink.h>
#include <ttyd/battle_database_common.h>

void hgf_init();
void hgf_oslink_hook(gc::OSLink::OSModuleInfo *);
void hgf_osunlink_hook(gc::OSLink::OSModuleInfo *);
void hgf_load_custom_rel();
void hgf_patch();
void hgf_free();