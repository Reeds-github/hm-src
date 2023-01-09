#pragma once

#include <cstdint>

namespace ttyd::battle_menu_disp {

extern "C" {

// .text
// DrawMultiItemWin
// BattleMenuDisp_MultiItem_End
// BattleMenuDisp_MultiItem_Disp
// BattleMenuDisp_MultiItem_Main
// BattleMenuDisp_MultiItem_Init
// DrawChangePartyWin
// BattleMenuDisp_ChangePartySelect_End
// BattleMenuDisp_ChangePartySelect_Disp
// BattleMenuDisp_ChangePartySelect_Main
// BattleMenuDisp_ChangePartySelect_Init
// DrawOperationWin
// BattleMenuDisp_Operation_End
// BattleMenuDisp_Operation_Disp
// BattleMenuDisp_Operation_Main
// BattleMenuDisp_Operation_Init
// DrawWeaponWin
// BattleMenuDisp_WeaponSelect_End
// BattleMenuDisp_WeaponSelect_Disp
// BattleMenuDisp_WeaponSelect_Main
// BattleMenuDisp_WeaponSelect_Init
// DrawMainMenu
// BattleMenuDisp_ActSelect_End
// BattleMenuDisp_ActSelect_Disp
// BattleMenuDisp_ActSelect_Main
// BattleMenuDisp_ActSelect_Init
// DrawSubMenuCommonProcess
// DrawSubMenuCommonProcessSub1
// InitSubMenuCommonProcess3
// InitSubMenuCommonProcess2
// InitSubMenuCommonProcess
// DrawMenuCursorAndScrollArrow
// DrawMenuHelpWin
// DrawMenuPinchMark
// N_DrawMenuPartyPinchMark
// N_DrawMenuMarioPinchMark
// DrawMenuPartyChangeButton_Sub
// DrawMenuPartyChangeButton
void DrawSubIconSub(int32_t unk1, int32_t unk2, int32_t unk3, int32_t unk4, uint16_t iconId, const char* name, int32_t unk7, int32_t unk8, int32_t unk9, int32_t unk10);
// DrawSubIcon
// DrawMainIconCircle
// DrawMainIconHukidasi
// DrawMainIcon
// GetRingOffset
// GetRingCenter
// battleMenuDispEnd
// battleMenuDispInit
// BattleMenuKeyOKInACT
// SelectedItemCoordinateColorUpDate

// .data
// kakutable
// dat_ptrarr_80363614
// dat_ptrarr_80363644
// dat_ptrarr_803636e8

// .sdata
// seleItemCoordCol
// danger_icon_tbl
// peril_icon_tbl

// .sbss
// seq$510

// .sbss2
// unk_80429598

}

}