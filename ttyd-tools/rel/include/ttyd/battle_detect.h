#pragma once

#include <ttyd/battle.h>

#include <cstdint>

namespace ttyd::battle_detect {

extern "C" {

void BattleGetFirstAttackUnit(ttyd::battle::BattleWork* battleWork, ttyd::battle_unit::BattleWorkUnit** unit1, ttyd::battle_unit::BattleWorkUnit** unit2);
// BattleChoiceSamplingEnemy
// BattleSamplingEnemyUpdate
// BattleSamplingEnemy
// _btlSamplingEnemy

}

}