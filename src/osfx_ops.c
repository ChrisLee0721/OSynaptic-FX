#include "../include/osfx_ops.h"

#include <string.h>

/*
 * Compact Device Operations table — derived from
 * Opensynaptic_Ucum_Device_Operations.json v1.1.0 (class_id 0x0E).
 *
 * Core section (19 entries): always compiled.
 * Extended section (52 entries: cmdA-Z + modeA-Z): compiled only when
 *   OSFX_OPS_INCLUDE_EXTENDED is 1 (the default).
 *
 * Flash cost estimate (ARM Cortex-M, -Os):
 *   Core only  : ~228 bytes (table) + ~120 bytes (functions) ≈ 350 B
 *   Full build : ~852 bytes (table) + ~120 bytes (functions) ≈ 1 KB
 */

/* ------------------------------------------------------------------
 * Static table
 * ------------------------------------------------------------------ */
static const osfx_op_record OSFX_OPS_TABLE[] = {
    /* --- Core operations ------------------------------------------ */
    { "cmd",    OSFX_TID_CMD,    0 },
    { "pow.on", OSFX_TID_POW_ON, 0 },
    { "pow.off",OSFX_TID_POW_OFF,0 },
    { "set.val",OSFX_TID_SET_VAL,OSFX_OP_F_REQUIRES_VALUE },
    { "get.val",OSFX_TID_GET_VAL,OSFX_OP_F_READ },
    { "get.st", OSFX_TID_GET_ST, OSFX_OP_F_READ },
    { "rst",    OSFX_TID_RST,    0 },
    { "mv.up",  OSFX_TID_MV_UP,  0 },
    { "mv.dn",  OSFX_TID_MV_DN,  0 },
    { "mv.lt",  OSFX_TID_MV_LT,  0 },
    { "mv.rt",  OSFX_TID_MV_RT,  0 },
    { "mv.fw",  OSFX_TID_MV_FW,  0 },
    { "mv.bk",  OSFX_TID_MV_BK,  0 },
    { "stp",    OSFX_TID_STP,    0 },
    { "stp.e",  OSFX_TID_STP_E,  0 },
    { "mv.to",  OSFX_TID_MV_TO,  OSFX_OP_F_REQUIRES_VALUE },
    { "mv.by",  OSFX_TID_MV_BY,  OSFX_OP_F_REQUIRES_VALUE },
    { "rot.cw", OSFX_TID_ROT_CW, 0 },
    { "rot.cc", OSFX_TID_ROT_CC, 0 },

#if OSFX_OPS_INCLUDE_EXTENDED
    /* --- Custom command slots A–Z --------------------------------- */
    { "cmdA",   OSFX_TID_CMD_A,  0 },
    { "cmdB",   OSFX_TID_CMD_B,  0 },
    { "cmdC",   OSFX_TID_CMD_C,  0 },
    { "cmdD",   OSFX_TID_CMD_D,  0 },
    { "cmdE",   OSFX_TID_CMD_E,  0 },
    { "cmdF",   OSFX_TID_CMD_F,  0 },
    { "cmdG",   OSFX_TID_CMD_G,  0 },
    { "cmdH",   OSFX_TID_CMD_H,  0 },
    { "cmdI",   OSFX_TID_CMD_I,  0 },
    { "cmdJ",   OSFX_TID_CMD_J,  0 },
    { "cmdK",   OSFX_TID_CMD_K,  0 },
    { "cmdL",   OSFX_TID_CMD_L,  0 },
    { "cmdM",   OSFX_TID_CMD_M,  0 },
    { "cmdN",   OSFX_TID_CMD_N,  0 },
    { "cmdO",   OSFX_TID_CMD_O,  0 },
    { "cmdP",   OSFX_TID_CMD_P,  0 },
    { "cmdQ",   OSFX_TID_CMD_Q,  0 },
    { "cmdR",   OSFX_TID_CMD_R,  0 },
    { "cmdS",   OSFX_TID_CMD_S,  0 },
    { "cmdT",   OSFX_TID_CMD_T,  0 },
    { "cmdU",   OSFX_TID_CMD_U,  0 },
    { "cmdV",   OSFX_TID_CMD_V,  0 },
    { "cmdW",   OSFX_TID_CMD_W,  0 },
    { "cmdX",   OSFX_TID_CMD_X,  0 },
    { "cmdY",   OSFX_TID_CMD_Y,  0 },
    { "cmdZ",   OSFX_TID_CMD_Z,  0 },
    /* --- Mode select slots A–Z ------------------------------------ */
    { "modeA",  OSFX_TID_MODE_A, 0 },
    { "modeB",  OSFX_TID_MODE_B, 0 },
    { "modeC",  OSFX_TID_MODE_C, 0 },
    { "modeD",  OSFX_TID_MODE_D, 0 },
    { "modeE",  OSFX_TID_MODE_E, 0 },
    { "modeF",  OSFX_TID_MODE_F, 0 },
    { "modeG",  OSFX_TID_MODE_G, 0 },
    { "modeH",  OSFX_TID_MODE_H, 0 },
    { "modeI",  OSFX_TID_MODE_I, 0 },
    { "modeJ",  OSFX_TID_MODE_J, 0 },
    { "modeK",  OSFX_TID_MODE_K, 0 },
    { "modeL",  OSFX_TID_MODE_L, 0 },
    { "modeM",  OSFX_TID_MODE_M, 0 },
    { "modeN",  OSFX_TID_MODE_N, 0 },
    { "modeO",  OSFX_TID_MODE_O, 0 },
    { "modeP",  OSFX_TID_MODE_P, 0 },
    { "modeQ",  OSFX_TID_MODE_Q, 0 },
    { "modeR",  OSFX_TID_MODE_R, 0 },
    { "modeS",  OSFX_TID_MODE_S, 0 },
    { "modeT",  OSFX_TID_MODE_T, 0 },
    { "modeU",  OSFX_TID_MODE_U, 0 },
    { "modeV",  OSFX_TID_MODE_V, 0 },
    { "modeW",  OSFX_TID_MODE_W, 0 },
    { "modeX",  OSFX_TID_MODE_X, 0 },
    { "modeY",  OSFX_TID_MODE_Y, 0 },
    { "modeZ",  OSFX_TID_MODE_Z, 0 },
#endif /* OSFX_OPS_INCLUDE_EXTENDED */
};

#define OSFX_OPS_TABLE_SIZE \
    (sizeof(OSFX_OPS_TABLE) / sizeof(OSFX_OPS_TABLE[0]))

/* ------------------------------------------------------------------
 * API implementation
 * ------------------------------------------------------------------ */

size_t osfx_ops_count(void) {
    return OSFX_OPS_TABLE_SIZE;
}

const osfx_op_record* osfx_ops_find_by_ucum(const char* ucum_code) {
    size_t i;
    if (!ucum_code) {
        return NULL;
    }
    for (i = 0; i < OSFX_OPS_TABLE_SIZE; ++i) {
        if (strcmp(ucum_code, OSFX_OPS_TABLE[i].ucum) == 0) {
            return &OSFX_OPS_TABLE[i];
        }
    }
    return NULL;
}

const osfx_op_record* osfx_ops_find_by_tid(uint16_t tid) {
    size_t i;
    for (i = 0; i < OSFX_OPS_TABLE_SIZE; ++i) {
        if (tid == OSFX_OPS_TABLE[i].tid) {
            return &OSFX_OPS_TABLE[i];
        }
    }
    return NULL;
}
