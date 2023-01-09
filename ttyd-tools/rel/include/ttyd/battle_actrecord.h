#pragma once

#include <cstdint>

namespace ttyd::battle_actrecord {

extern "C" {

// .text
// _check_turn_count_0_end
// _check_turn_count_0_turn
// _check_no_use
// _check_use
// BtlActRec_JudgeRuleKeep
// BtlActRec_JudgeTurnRuleKeep
// BtlActRec_AddPoint
void BtlActRec_AddCount(uint8_t* counter);

// .data
// dat_ptrarr_803977b0

}

}