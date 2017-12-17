/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *       A amateur remote control software library. Use at your own risk.
 *
 * @file    pin_change.h
 * @brief   Input pin change initialization function and interrupt handler.
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef PIN_CHANGE_H_
#define PIN_CHANGE_H_


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

#define PC_GRP_0_IO_VALUE()     PINB
#define PC_GRP_1_IO_VALUE()     PINC
#define PC_GRP_2_IO_VALUE()     PIND


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */


typedef enum pc_pin_idx{
    /* Pin change group 0 */
    PC_PIN_IDX_0        = 0,            /* PortB 0 */
    PC_PIN_IDX_1,                       /* PortB 1 */
    PC_PIN_IDX_2,                       /* PortB 2 */
    PC_PIN_IDX_3,                       /* PortB 3 */
    PC_PIN_IDX_4,                       /* PortB 4 */
    PC_PIN_IDX_5,                       /* PortB 5 */
    PC_PIN_IDX_6,                       /* PortB 6 */
    PC_PIN_IDX_7,                       /* PortB 7 */

    /* Pin change group 1 */
    PC_PIN_IDX_8,                       /* PortC 0 */
    PC_PIN_IDX_9,                       /* PortC 1 */
    PC_PIN_IDX_10,                      /* PortC 2 */
    PC_PIN_IDX_11,                      /* PortC 3 */
    PC_PIN_IDX_12,                      /* PortC 4 */
    PC_PIN_IDX_13,                      /* PortC 5 */
    PC_PIN_IDX_14,                      /* PortC 6 */
    PC_PIN_IDX_UNUSED0,                 /* Unused 0 */

    /* Pin change group 2 */
    PC_PIN_IDX_16,                      /* PortD 0 */
    PC_PIN_IDX_17,                      /* PortD 1 */
    PC_PIN_IDX_18,                      /* PortD 2 */
    PC_PIN_IDX_19,                      /* PortD 3 */
    PC_PIN_IDX_20,                      /* PortD 4 */
    PC_PIN_IDX_21,                      /* PortD 5 */
    PC_PIN_IDX_22,                      /* PortD 6 */
    PC_PIN_IDX_23,                      /* PortD 7 */
}__attribute__((packed)) PC_PIN_IDX;

typedef enum pc_pin_mask{
    /* Pin change group 0 */
    PC_PIN_MASK_0       = _BV(PCINT0),  /* PortB 0 */
    PC_PIN_MASK_1       = _BV(PCINT1),  /* PortB 1 */
    PC_PIN_MASK_2       = _BV(PCINT2),  /* PortB 2 */
    PC_PIN_MASK_3       = _BV(PCINT3),  /* PortB 3 */
    PC_PIN_MASK_4       = _BV(PCINT4),  /* PortB 4 */
    PC_PIN_MASK_5       = _BV(PCINT5),  /* PortB 5 */
    PC_PIN_MASK_6       = _BV(PCINT6),  /* PortB 6 */
    PC_PIN_MASK_7       = _BV(PCINT7),  /* PortB 7 */

    /* Pin change group 1 */
    PC_PIN_MASK_8       = _BV(PCINT8),  /* PortC 0 */
    PC_PIN_MASK_9       = _BV(PCINT9),  /* PortC 1 */
    PC_PIN_MASK_10      = _BV(PCINT10), /* PortC 2 */
    PC_PIN_MASK_11      = _BV(PCINT11), /* PortC 3 */
    PC_PIN_MASK_12      = _BV(PCINT12), /* PortC 4 */
    PC_PIN_MASK_13      = _BV(PCINT13), /* PortC 5 */
    PC_PIN_MASK_14      = _BV(PCINT14), /* PortC 6 */
    PC_PIN_MASK_UNUSED0 = 0,            /* Unused 0 */

    /* Pin change group 2 */
    PC_PIN_MASK_16      = _BV(PCINT16), /* PortD 0 */
    PC_PIN_MASK_17      = _BV(PCINT17), /* PortD 1 */
    PC_PIN_MASK_18      = _BV(PCINT18), /* PortD 2 */
    PC_PIN_MASK_19      = _BV(PCINT19), /* PortD 3 */
    PC_PIN_MASK_20      = _BV(PCINT20), /* PortD 4 */
    PC_PIN_MASK_21      = _BV(PCINT21), /* PortD 5 */
    PC_PIN_MASK_22      = _BV(PCINT22), /* PortD 6 */
    PC_PIN_MASK_23      = _BV(PCINT23), /* PortD 7 */
}__attribute__((packed)) PC_PIN_MASK;

typedef enum pc_grp_idx{
    PC_GRP_IDX_0        = 0,            /* Pin group 0, Port B */
    PC_GRP_IDX_1,                       /* Pin group 1, Port C */
    PC_GRP_IDX_2,                       /* Pin group 2, Port D */
    PC_GRP_TOTAL        = 3,
}__attribute__((packed)) PC_GRP_IDX;

typedef enum pc_grp_shift{
    PC_GRP_SHIFT_0      = 0,
    PC_GRP_SHIFT_1      = 8,
    PC_GRP_SHIFT_2      = 16,
}__attribute__((packed)) PC_GRP_SHIFT;



/*
 *******************************************************************************
 * Global variables
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Public functions declaration
 *******************************************************************************
 */

int8_t PC_Init();
int8_t PC_Setup(PC_GRP_IDX pc_grp_idx, uint8_t pc_pin_mask, bool is_enable);
void PC_QuickEnable(PC_GRP_IDX pc_grp_idx, uint8_t pc_pin_mask);
void PC_QuickDisable(PC_GRP_IDX pc_grp_idx, uint8_t pc_pin_mask);


/*
 *******************************************************************************
 * Private functions declaration
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * function_example - Function example
 *
 * @param   [in]    input       Example input.
 * @param   [out]   *p_output   Example output.
 *
 * @return  [int]   Function executing result.
 * @retval  [0]     Success.
 * @retval  [-1]    Fail.
 *
 */


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

#endif // PIN_CHANGE_H_
