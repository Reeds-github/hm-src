#include "mod.h"
#include "patch.h"

#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>

#include <hm/bugpatches.h>
#include <hm/common.h>

using namespace mod;
using namespace ttyd;
using namespace ttyd::item_data;

// these patches are from Zephiles' Practice Codes
void bugpatches_init() {
    patch::writeBranch((void *)0x80122ba4, (void *)StartDisplayMegaJumpBadgeBattleMenu, true);
    patch::writeBranch((void *)0x80122bb8, (void *)StartDisplayMegaHammerBadgesBattleMenu, true);
    patch::writeBranch((void *)0x8010f810, (void *)StartFixBlooperCrash1, true);
    patch::writeBranch((void *)0x8010f888, (void *)StartFixBlooperCrash2, true);
}

bool displayMegaJumpBadgeInMenu(uint32_t checkBit) {
    if (mario_pouch::pouchEquipCheckBadge(ItemType::MEGA_JUMP) > 0) {
        return false;
    } else {
        return (checkBit & (1 << 9));  // Check if the 9 bit is on
    }
}

bool displayMegaHammerBadgesInMenu(uint32_t checkBit) {
    if (mario_pouch::pouchEquipCheckBadge(ItemType::MEGA_SMASH) > 0 ||
        mario_pouch::pouchEquipCheckBadge(ItemType::MEGA_QUAKE) > 0) {
        return false;
    } else {
        return (checkBit & (1 << 10));  // Check if the 10 bit is on
    }
}

uint32_t fixBlooperCrash1(uint32_t unkValue, void *battleUnitPointer) {
    if (isPointerValid(battleUnitPointer)) {
        *reinterpret_cast<uint32_t *>(reinterpret_cast<uint32_t>(battleUnitPointer) + 0x218) = unkValue;
    }

    return 2;
}