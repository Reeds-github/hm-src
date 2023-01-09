#pragma once

#include <cstdint>

namespace ttyd::eff_starstone {
    
extern "C" {

// .text
// effStarStoneDraw
// effStarStoneDisp_2
// effStarStoneDisp_1
// effStarStoneMain
// effStarStoneEntry

// .data
extern float edge_color[7][3];

// .sdata
extern void* color_tbl[7];

}

}