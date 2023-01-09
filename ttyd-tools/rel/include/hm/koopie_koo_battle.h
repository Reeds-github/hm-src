#pragma once

#include <gc/OSLink.h>
#include <ttyd/battle_database_common.h>

void kkb_init();
void kkb_oslink_hook(gc::OSLink::OSModuleInfo *);
void kkb_osunlink_hook(gc::OSLink::OSModuleInfo *);
const char* kkb_filealloc_hook(const char *);
const char* kkb_msgsearch_hook(const char *);
void kkb_unitentry_hook(ttyd::battle_database_common::BattleUnitSetup *);
void kkb_load_custom_rel();
void kkb_patch();
void kkb_free();