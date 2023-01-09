#pragma once

#include <ttyd/battle.h>

#include <cstdint>

namespace ttyd::battle_mario {

extern "C" {

// .text
// _paper_light_off
// _battle_majinai_effect
// _majinai_powerup_check
// _fire_wave
// _mario_fire_ball_controll
// _kaiten_hammer_acrobat_rotate
// _hammer_star_effect
// mario_get_renzoku_count_max
// _record_renzoku_count
// _tatsumaki_effect
// _wait_jyabara_hit_iron_frame
// _bgset_iron_frame_check
// _jump_star_effect
// _whirlwind_effect
// _get_mario_hammer_lv

// .data
// marioDefaultWeapon_Jump
// marioWeapon_KururinJump
// marioWeapon_KururinJump2
// marioWeapon_JyabaraJump
// marioWeapon_JyabaraJumpFailed
// marioDefaultWeapon_FirstAttackKaitenHammer
// marioDefaultWeapon_FirstAttackUltraHammer
// marioDefaultWeapon_Hammer
// marioWeapon_KaitenHammer
// marioWeapon_KaitenHammerInvolved
// marioWeapon_UltraHammer
// marioWeapon_UltraHammerInvolved
// marioWeapon_UltraHammerFinish
// badgeWeapon_TugitugiJump
// badgeWeapon_GatsunJump
// badgeWeapon_DokanJump
// badgeWeapon_RenzokuJump
extern battle_database_common::BattleWeapon badgeWeapon_TatsumakiJump;
// badgeWeapon_TatsumakiJumpInvolved
// badgeWeapon_MiniminiFumi
// badgeWeapon_NemuraseFumi
// badgeWeapon_FunyafunyaJump
// badgeWeapon_GatsunHammer
// badgeWeapon_DokanHammer
// badgeWeapon_JishinHammer
// badgeWeapon_UltraJishinHammer
// badgeWeapon_HammerNageru
// badgeWeapon_TsuranukiNaguri
// badgeWeapon_ConfuseHammer
// badgeWeapon_FireNaguri
// badgeWeapon_FireNaguriFailed
// badgeWeapon_IceNaguri
// badgeWeapon_Charge
// badgeWeapon_ChargeP
// badgeWeapon_SuperCharge
// badgeWeapon_SuperChargeP
// badgeWeapon_2KaiItem
// badgeWeapon_3KaiItem
// badgeWeapon_DefenceCommand
// badgeWeapon_DefenceCommandP
// marioWeapon_BakuGame
// marioWeapon_Scissor
// marioWeapon_ZubaStar
// marioWeapon_Genki0
// marioWeapon_Genki1
// marioWeapon_Deka
// marioWeapon_Muki
// marioWeapon_Suki
extern battle_database_common::BattleWeapon* superActionTable[8];
// marioAttackEvent_NormalJump
// marioAttackEvent_KururinJump
// marioAttackEvent_JyabaraJump
// marioAttackEvent_GatsuDokaJump
// marioAttackEvent_TatsumakiJump
// marioAttackEvent_RenzokuJump
// marioAttackEvent_TugiTugiJump
// marioAttackEvent_NormalHammer_Core
// marioAttackEvent_KaitenHammer
// marioAttackEvent_FirstAttackKaitenHammer
// marioAttackEvent_JishinHammer
// marioAttackEvent_UltraJishinHammer
// marioAttackEvent_HammerRotate
// marioAttackEvent_HammerNageru
// marioAttackEvent_HammerNageru_object
// marioAttackEvent_FireNaguri
// marioAttackEvent_FireNaguri_object
// _NormalCharge_core
// _SuperCharge_core
// marioAttackEvent_MajinaiPowerUpCheck
// marioAttackEvent_MajinaiJump
// _MajinaiPoseReset
// marioAttackEvent_MajinaiDefenceUp
// marioAttackEvent_MajinaiExpUp

// .sdata
// marioAttackEvent_GatsunJump
// marioAttackEvent_DokanJump
// marioAttackEvent_MiniminiFumi
// marioAttackEvent_NemuraseFumi
// marioAttackEvent_FunyafunyaJump
// marioAttackEvent_NormalHammer
// marioAttackEvent_GatsunHammer
// marioAttackEvent_IceNaguri
// marioAttackEvent_TsuranukiHammer
// marioAttackEvent_NormalCharge
// marioAttackEvent_NormalChargeP
// marioAttackEvent_SuperCharge
// marioAttackEvent_SuperChargeP

}

}