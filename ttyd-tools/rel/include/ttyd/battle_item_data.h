#pragma once

#include <ttyd/battle_database_common.h>

#include <cstdint>

using ttyd::battle_database_common::BattleWeapon;

namespace ttyd::battle_item_data {

extern "C" {

// .text
// _full_ap_recover
// _nancy_disp
// _nancy_mirage
// _disp_item_icon
// _throw_item_icon
// _get_throw_first_pos
// _naniga_okorukana_check
// _hpfp_change_declare_2
// _hpfp_change_declare_1
// _rank_up
// _cam_rot
// _sampling_random_target

// .data
extern int32_t _return_home_event[];
// ItemEvent_Automatic_Target
// ItemEvent_Recovery_Core_Effect
// ItemEvent_Recovery_Core
// ItemEvent_Recovery
// ItemEvent_Support_Sub_UseDeclere
// ItemEvent_GetTarget
extern int32_t ItemEvent_Support_Sub_Effect[];
// ItemEvent_Support_NoEffect
// ItemEvent_Support_RefreshEffect
// ItemWeaponData_CookingItem
// ItemWeaponData_Kinoko
// ItemWeaponData_Kinkyuu_Kinoko
// ItemWeaponData_Super_Kinoko
// ItemWeaponData_Ultra_Kinoko
// ItemWeaponData_Sinabita_Kinoko
// ItemWeaponData_Honey_Syrup
// ItemWeaponData_Maple_Syrup
// ItemWeaponData_Royal_Jelly
// ItemWeaponData_Tankobu
extern int32_t ItemEvent_Pow_Block[];
extern BattleWeapon ItemWeaponData_Pow_Block;
extern int32_t ItemEvent_Fire_Flower_Core[];
extern int32_t ItemEvent_Fire_Flower[];
extern BattleWeapon ItemWeaponData_Fire_Flower;
extern int32_t ItemEvent_Kaminari_Core[];
// ItemEvent_Kaminari
// ItemWeaponData_Kaminari_Dokkan
// ItemWeaponData_Kaminari_Gorogoro
extern int32_t ItemEvent_Kirakira_Otoshi_Core[];
// ItemEvent_Kirakira_Otoshi
// ItemWeaponData_Kirakira_Otoshi
extern int32_t ItemEvent_Koorino_Ibuki[];
// ItemWeaponData_Koorino_Ibuki
// ItemEvent_Yurayura_Jishin
// ItemWeaponData_Yurayura_Jishin
// ItemEvent_Teresano_Fuku
// ItemWeaponData_Teresano_Fuku
// ItemEvent_Biribiri_Kinoko_Core
// ItemEvent_Biribiri_Kinoko
// ItemWeaponData_Biribiri_Kinoko
// ItemEvent_Teki_Yokeeru_Core
// ItemEvent_Teki_Yokeeru
// ItemWeaponData_Teki_Yokeeru
// ItemWeaponData_Madowaseno_Kona
// ItemEvent_Nemure_Yoikoyo
extern BattleWeapon ItemWeaponData_Nemure_Yoikoyo;
// ItemEvent_Stop_Watch_Core
// ItemEvent_Stop_Watch
// ItemWeaponData_Stop_Watch
// ItemEvent_Guruguru_Memawashi
extern BattleWeapon ItemWeaponData_Guruguru_Memawashi;
// ItemWeaponData_Dekadeka_Drink
// ItemWeaponData_Katikati_Koura
// ItemEvent_Suitooru
// ItemWeaponData_Suitooru
// ItemEvent_Teki_Kyouka
// ItemWeaponData_Teki_Kyouka
// ItemWeaponData_Minimini_Kun
// ItemWeaponData_Funyafunya_Kun
// ItemWeaponData_Sukkiri_Drink
// ItemEvent_Jiwajiwa_Kinoko_Core
extern int32_t ItemEvent_Jiwajiwa_Kinoko[];
// ItemWeaponData_Jiwajiwa_Kinoko
// ItemEvent_Jiwajiwa_Syrup_Core
// ItemEvent_Jiwajiwa_Syrup
// ItemWeaponData_Jiwajiwa_Syrup
// ItemEvent_Irekaeeru
// ItemWeaponData_Irekaeeru
// ItemEvent_Attiike_Shisshi
// ItemWeaponData_Attiike_Shisshi
// ItemEvent_Naniga_Okorukana
// ItemWeaponData_Naniga_Okorukana
// R_mystery_item_table
// ItemWeaponData_Shikaeshino_Kona
// ItemWeaponData_Kameno_Noroi
// ItemEvent_RecoveryAndStatus
// ItemWeaponData_SpaceFood
// ItemWeaponData_IceCandy
// ItemWeaponData_NancyFrappe
// ItemWeaponData_SnowRabbit
// ItemWeaponData_TeaKinoko
// ItemWeaponData_FirstLovePuddingBiribiri
// ItemWeaponData_FirstLovePuddingTrans
// ItemWeaponData_FirstLovePuddingSleep
// ItemWeaponData_StarryDinner
// ItemWeaponData_KararinaPasta
// ItemWeaponData_MeromeroCake
// ItemWeaponData_PeachTaltBiribiri
// ItemWeaponData_PeachTaltDodge
// ItemWeaponData_PeachTaltSleep
// ItemWeaponData_BiribiriCandy
// ItemWeaponData_HealthySalad
// ItemWeaponData_FreshJuice
// ItemWeaponData_RedKararing
// ItemWeaponData_FutarideForever
// ItemEvent_ThrowAttack
// ItemWeaponData_NancyDynamite
// ItemWeaponData_CoconutsBomb
// ItemWeaponData_KachikachiDish
// ItemWeaponData_BomberEgg
// ItemEvent_Poison_Kinoko
// ItemWeaponData_PoisonKinoko
// ItemEvent_LastDinner
// ItemWeaponData_LastDinner

// .sdata
// ItemEvent_Kaminari_Dokkan
// ItemEvent_Kaminari_Gorogoro
// ItemEvent_Madowaseno_Kona
extern int32_t ItemEvent_Dekadeka_Drink[];
// ItemEvent_Katikati_Koura
// ItemEvent_Minimini_Kun
// ItemEvent_Funyafunya_Kun
// ItemEvent_Sukkiri_Drink
// ItemEvent_Shikaeshino_Kona
// ItemEvent_Kameno_Noroi

}

}