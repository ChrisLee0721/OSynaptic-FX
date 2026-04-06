#ifndef OSFX_OPS_H
#define OSFX_OPS_H

/*
 * osfx_ops.h — MCU-side Device Operations table
 * -----------------------------------------------
 * Reduced C representation of Opensynaptic_Ucum_Device_Operations.json
 * (class_id 0x0E, version 1.1.0).
 *
 * Provides:
 *  - OSFX_TID_* constants for all operation TIDs (no RAM cost).
 *  - Compact osfx_op_record table for runtime ucum-code / TID lookup.
 *  - Compile-time size control: define OSFX_OPS_INCLUDE_EXTENDED 0
 *    to exclude the cmdA-Z / modeA-Z reserved slots and save ~1 KB flash.
 *
 * Default: OSFX_OPS_INCLUDE_EXTENDED 1 (all 71 operations included).
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------
 * Build control (override in osfx_user_config.h or compiler flags)
 * ------------------------------------------------------------------ */
#ifndef OSFX_OPS_INCLUDE_EXTENDED
#define OSFX_OPS_INCLUDE_EXTENDED 1
#endif

/* ------------------------------------------------------------------
 * TID constants — class 0x0E
 * ------------------------------------------------------------------ */

/* Core operations (0x0E00 – 0x0E12) */
#define OSFX_TID_CMD        ((uint16_t)0x0E00U)  /* raw command            */
#define OSFX_TID_POW_ON     ((uint16_t)0x0E01U)  /* power on               */
#define OSFX_TID_POW_OFF    ((uint16_t)0x0E02U)  /* power off              */
#define OSFX_TID_SET_VAL    ((uint16_t)0x0E03U)  /* set value (w/ value)   */
#define OSFX_TID_GET_VAL    ((uint16_t)0x0E04U)  /* get value (read)       */
#define OSFX_TID_GET_ST     ((uint16_t)0x0E05U)  /* get status (read)      */
#define OSFX_TID_RST        ((uint16_t)0x0E06U)  /* reset / reboot         */
#define OSFX_TID_MV_UP      ((uint16_t)0x0E07U)  /* move up                */
#define OSFX_TID_MV_DN      ((uint16_t)0x0E08U)  /* move down              */
#define OSFX_TID_MV_LT      ((uint16_t)0x0E09U)  /* move left              */
#define OSFX_TID_MV_RT      ((uint16_t)0x0E0AU)  /* move right             */
#define OSFX_TID_MV_FW      ((uint16_t)0x0E0BU)  /* move forward           */
#define OSFX_TID_MV_BK      ((uint16_t)0x0E0CU)  /* move backward          */
#define OSFX_TID_STP        ((uint16_t)0x0E0DU)  /* stop                   */
#define OSFX_TID_STP_E      ((uint16_t)0x0E0EU)  /* emergency stop         */
#define OSFX_TID_MV_TO      ((uint16_t)0x0E0FU)  /* move to abs (w/ value) */
#define OSFX_TID_MV_BY      ((uint16_t)0x0E10U)  /* move by rel (w/ value) */
#define OSFX_TID_ROT_CW     ((uint16_t)0x0E11U)  /* rotate clockwise       */
#define OSFX_TID_ROT_CC     ((uint16_t)0x0E12U)  /* rotate counter-CW      */

/* Extended: custom command slots A–Z (0x0E13 – 0x0E2C) */
#define OSFX_TID_CMD_A      ((uint16_t)0x0E13U)
#define OSFX_TID_CMD_B      ((uint16_t)0x0E14U)
#define OSFX_TID_CMD_C      ((uint16_t)0x0E15U)
#define OSFX_TID_CMD_D      ((uint16_t)0x0E16U)
#define OSFX_TID_CMD_E      ((uint16_t)0x0E17U)
#define OSFX_TID_CMD_F      ((uint16_t)0x0E18U)
#define OSFX_TID_CMD_G      ((uint16_t)0x0E19U)
#define OSFX_TID_CMD_H      ((uint16_t)0x0E1AU)
#define OSFX_TID_CMD_I      ((uint16_t)0x0E1BU)
#define OSFX_TID_CMD_J      ((uint16_t)0x0E1CU)
#define OSFX_TID_CMD_K      ((uint16_t)0x0E1DU)
#define OSFX_TID_CMD_L      ((uint16_t)0x0E1EU)
#define OSFX_TID_CMD_M      ((uint16_t)0x0E1FU)
#define OSFX_TID_CMD_N      ((uint16_t)0x0E20U)
#define OSFX_TID_CMD_O      ((uint16_t)0x0E21U)
#define OSFX_TID_CMD_P      ((uint16_t)0x0E22U)
#define OSFX_TID_CMD_Q      ((uint16_t)0x0E23U)
#define OSFX_TID_CMD_R      ((uint16_t)0x0E24U)
#define OSFX_TID_CMD_S      ((uint16_t)0x0E25U)
#define OSFX_TID_CMD_T      ((uint16_t)0x0E26U)
#define OSFX_TID_CMD_U      ((uint16_t)0x0E27U)
#define OSFX_TID_CMD_V      ((uint16_t)0x0E28U)
#define OSFX_TID_CMD_W      ((uint16_t)0x0E29U)
#define OSFX_TID_CMD_X      ((uint16_t)0x0E2AU)
#define OSFX_TID_CMD_Y      ((uint16_t)0x0E2BU)
#define OSFX_TID_CMD_Z      ((uint16_t)0x0E2CU)

/* Extended: mode select slots A–Z (0x0E2D – 0x0E46) */
#define OSFX_TID_MODE_A     ((uint16_t)0x0E2DU)
#define OSFX_TID_MODE_B     ((uint16_t)0x0E2EU)
#define OSFX_TID_MODE_C     ((uint16_t)0x0E2FU)
#define OSFX_TID_MODE_D     ((uint16_t)0x0E30U)
#define OSFX_TID_MODE_E     ((uint16_t)0x0E31U)
#define OSFX_TID_MODE_F     ((uint16_t)0x0E32U)
#define OSFX_TID_MODE_G     ((uint16_t)0x0E33U)
#define OSFX_TID_MODE_H     ((uint16_t)0x0E34U)
#define OSFX_TID_MODE_I     ((uint16_t)0x0E35U)
#define OSFX_TID_MODE_J     ((uint16_t)0x0E36U)
#define OSFX_TID_MODE_K     ((uint16_t)0x0E37U)
#define OSFX_TID_MODE_L     ((uint16_t)0x0E38U)
#define OSFX_TID_MODE_M     ((uint16_t)0x0E39U)
#define OSFX_TID_MODE_N     ((uint16_t)0x0E3AU)
#define OSFX_TID_MODE_O     ((uint16_t)0x0E3BU)
#define OSFX_TID_MODE_P     ((uint16_t)0x0E3CU)
#define OSFX_TID_MODE_Q     ((uint16_t)0x0E3DU)
#define OSFX_TID_MODE_R     ((uint16_t)0x0E3EU)
#define OSFX_TID_MODE_S     ((uint16_t)0x0E3FU)
#define OSFX_TID_MODE_T     ((uint16_t)0x0E40U)
#define OSFX_TID_MODE_U     ((uint16_t)0x0E41U)
#define OSFX_TID_MODE_V     ((uint16_t)0x0E42U)
#define OSFX_TID_MODE_W     ((uint16_t)0x0E43U)
#define OSFX_TID_MODE_X     ((uint16_t)0x0E44U)
#define OSFX_TID_MODE_Y     ((uint16_t)0x0E45U)
#define OSFX_TID_MODE_Z     ((uint16_t)0x0E46U)

/* ------------------------------------------------------------------
 * Operation flags (osfx_op_record.flags)
 * ------------------------------------------------------------------ */
#define OSFX_OP_F_REQUIRES_VALUE  ((uint8_t)0x01U)  /* command carries a value */
#define OSFX_OP_F_READ            ((uint8_t)0x02U)  /* read (device→host) direction */

/* ------------------------------------------------------------------
 * Compact operation record — 3 fields, no conversion factor needed
 * ------------------------------------------------------------------ */
typedef struct osfx_op_record {
    const char* ucum;   /* UCUM code string, e.g. "pow.on"  */
    uint16_t    tid;    /* TID from class 0x0E table        */
    uint8_t     flags;  /* OSFX_OP_F_* bitmask              */
} osfx_op_record;

/* ------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------ */

/**
 * osfx_ops_count()
 * Returns the number of entries compiled into the table.
 * Core-only build (OSFX_OPS_INCLUDE_EXTENDED 0): 19
 * Full build (OSFX_OPS_INCLUDE_EXTENDED 1):       71
 */
size_t osfx_ops_count(void);

/**
 * osfx_ops_find_by_ucum()
 * Look up an operation by UCUM code (case-sensitive, exact match).
 * Returns a pointer into the static table, or NULL if not found.
 */
const osfx_op_record* osfx_ops_find_by_ucum(const char* ucum_code);

/**
 * osfx_ops_find_by_tid()
 * Look up an operation by 16-bit TID.
 * Returns a pointer into the static table, or NULL if not found.
 */
const osfx_op_record* osfx_ops_find_by_tid(uint16_t tid);

#ifdef __cplusplus
}
#endif

#endif /* OSFX_OPS_H */
