#pragma once

extern "C" {

void StartDisplayMegaJumpBadgeBattleMenu();
void StartDisplayMegaHammerBadgesBattleMenu();
void StartFixBlooperCrash1();
void StartFixBlooperCrash2();

bool displayMegaJumpBadgeInMenu(uint32_t checkBit);
bool displayMegaHammerBadgesInMenu(uint32_t checkBit);
uint32_t fixBlooperCrash1(uint32_t unkValue, void *battleUnitPointer);

}

void bugpatches_init();