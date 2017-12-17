/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *      A amateur remote control software library. Use at your own risk.
 *
 * @file    OneRCAirplane.cpp
 * @brief
 *
 *          Abbreviations:
 *              DBG     - Debug             POT     - Potentiometer
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <Arduino.h>
#include <string.h>
#include <OneRCLib.h>

#include "OneRCAirplane.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

#define AIRPLANE_KP_CH                  ADC_CH0
#define AIRPLANE_KI_CH                  ADC_CH1
#define AIRPLANE_KD_CH                  ADC_CH2

#define AIRPLANE_ROLL_SCALE_CH          ADC_CH0
#define AIRPLANE_PITCH_SCALE_CH         ADC_CH1
#define AIRPLANE_YAW_SCALE_CH           ADC_CH2
#define AIRPLANE_NAV_LOITER_CH          ADC_CH3
#define AIRPLANE_MODE_DIPS_CH           ADC_CH4

#define AIRPLANE_DIP_RC_CALI_ACTIVE     0
#define AIRPLANE_DIP_RC_CALI_START      1

#define AIRPLANE_RC_CALI_NONE           0
#define AIRPLANE_RC_CALI_NEUTRAL        1
#define AIRPLANE_RC_CALI_RANGE          2
#define AIRPLANE_RC_CALI_FAILSAFE       3
#define AIRPLANE_RC_CALI_FINISH         4

/* Potentiometers */
#define AIRPLANE_PID_POT_COMMON         1       /*
                                                 * Using 3 potentiometers to adjust
                                                 * KP, KI, KD of roll, pitch and yaw
                                                 * stabilizing control.
                                                 */
#define AIRPLANE_PID_POT_SCALE          2       /*
                                                 * Using 3 potentiometers to adjust
                                                 * the PID scale parameter of roll,
                                                 * pitch and yaw stabilizing control
                                                 * separately.
                                                 */


#define AIRPLANE_PID_POT_EN             true    /* To enable the Potentiometers on FC */

#define AIRPLANE_PID_POT_TYPE           AIRPLANE_PID_POT_SCALE

#define AIRPLANE_WPT_ARRIVE_RADIUS      10.0    /* Meters */

#define AIRPLANE_GET_LOITER_RADIUS()    ((ADC_Read(AIRPLANE_NAV_LOITER_CH) * 0.20)  \
                                         + AIRPLANE_WPT_ARRIVE_RADIUS)

#define AIRPLANE_WPT_NUM                5


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */

typedef struct airplane_waypoint{
    bool is_actived;
    GPS_COORD_POINT wpt_coord;
}AIRPLANE_WAYPOINT;

typedef struct airplane_navigation{
    float loiter_radius;                    /* Loiter radius (meters) around home position */
    uint8_t current_wpt_idx;
    uint8_t total_wpt;
    AIRPLANE_WAYPOINT wpt[AIRPLANE_WPT_NUM];
}AIRPLANE_NAVIGATION;

typedef struct airplane_config{

    uint32_t config_ID;

    uint32_t config_date;

    AIRPLANE_TYPE model_type;

    bool is_imu_calibrated;
    int16_t accel_bias[IMU_AXES];
    int16_t gyro_bias[IMU_AXES];

    bool rc_in_is_reversed[RCIN_CH_TOTAL];
    uint16_t rc_in_neutral_ticks[RCIN_CH_TOTAL];
    uint16_t rc_in_max_ticks[RCIN_CH_TOTAL];
    uint16_t rc_in_min_ticks[RCIN_CH_TOTAL];
    uint16_t rc_in_failsafe_ticks[RCIN_CH_TOTAL];

    struct{
        float KP;
        float KI;
        float KD;
        float scale;
        float integral_max;
        float output_max;
    }pid_aile_cfg;

    struct{
        float KP;
        float KI;
        float KD;
        float scale;
        float integral_max;
        float output_max;
    }pid_elev_cfg;

    struct{
        float KP;
        float KI;
        float KD;
        float scale;
        float integral_max;
        float output_max;
    }pid_rudd_cfg;

    struct{
        float KP;
        float KI;
        float KD;
        float scale;
        float integral_max;
        float output_max;
    }pid_bank_turn_cfg;

    AIRPLANE_NAVIGATION navigation;

    uint16_t rom_crc16;

}AIRPLANE_CONFIG;

typedef struct airplane_status{
    AHRS_DATA ahrs_data;                    /* AHRS data */
    PID_DATA pid_aile_servo;                /* Ailerons servo PID data */
    PID_DATA pid_elev_servo;                /* Elevator servo PID data */
    PID_DATA pid_rudd_servo;                /* Rudder servo PID data */
    PID_DATA pid_band_turn;                 /* Bank turn PID data */

    struct{
        float roll_angle;
        float pitch_angle;
        float heading_angle;
    }setpoint;

    uint16_t rc_pulse_in[RCIN_CH_TOTAL];    /* RC pulse in */
    uint16_t rc_pulse_out[RCOUT_CH_TOTAL];  /* RC pulse out */

    struct{
        AIRPLANE_FLY_MODE fly_mode;

        uint8_t imu_fail_cnt;
        uint8_t ahrs_delay_cnt;
        uint8_t rcin_cyc_cnt;
        uint8_t rcout_cyc_cnt;

        uint16_t delta_ctrl_time;

        float mcu_vcc;

    }general;

    AIRPLANE_CRUISE_STATE current_cruise_state;

    uint32_t heartbeat;

}AIRPLANE_STATUS;


/*
 *******************************************************************************
 * Global variables
 *******************************************************************************
 */

/* Airplane configuration */
static AIRPLANE_CONFIG Airplane_Config =
{
    .config_ID = AIRPLANE_CONFIG_ID,                        /* "1_AP" */

    .config_date = AIRPLANE_FW_DATE,                        /* 2017/12/16 */

    /* default airplane control setting */
    .model_type = AIRPLANE_NORMAL,                          /* Normal airplane */

    .is_imu_calibrated = false,
    .accel_bias = {0, 0, 0},
    .gyro_bias = {0, 0, 0},

    /* default RC channel output value, give false or true for changing direction */
    .rc_in_is_reversed =
    {
        [RCIN_THRO_IDX] = false,
        [RCIN_AILE_IDX] = false,
        [RCIN_ELEV_IDX] = false,
        [RCIN_RUDD_IDX] = false,
        [RCIN_AUX1_IDX] = false,
    },

    /* default neutral point of each RCIN channel */
    .rc_in_neutral_ticks =
    {
        [RCIN_THRO_IDX] = TIMER1_MICROS_TO_TICKS(1500),     /* 1.5 ms by default */
        [RCIN_AILE_IDX] = TIMER1_MICROS_TO_TICKS(1500),     /* 1.5 ms by default */
        [RCIN_ELEV_IDX] = TIMER1_MICROS_TO_TICKS(1500),     /* 1.5 ms by default */
        [RCIN_RUDD_IDX] = TIMER1_MICROS_TO_TICKS(1500),     /* 1.5 ms by default */
        [RCIN_AUX1_IDX] = TIMER1_MICROS_TO_TICKS(1500),     /* 1.5 ms by default */
    },

    /* default maximum pulse ticks of each RCIN channel */
    .rc_in_max_ticks =
    {
        [RCIN_THRO_IDX] = TIMER1_MICROS_TO_TICKS(2000),     /* 2.0 ms by default */
        [RCIN_AILE_IDX] = TIMER1_MICROS_TO_TICKS(2000),     /* 2.0 ms by default */
        [RCIN_ELEV_IDX] = TIMER1_MICROS_TO_TICKS(2000),     /* 2.0 ms by default */
        [RCIN_RUDD_IDX] = TIMER1_MICROS_TO_TICKS(2000),     /* 2.0 ms by default */
        [RCIN_AUX1_IDX] = TIMER1_MICROS_TO_TICKS(2000),     /* 2.0 ms by default */
    },

    /* default minimum pulse ticks of each RCIN channel */
    .rc_in_min_ticks =
    {
        [RCIN_THRO_IDX] = TIMER1_MICROS_TO_TICKS(1000),     /* 1.0 ms by default */
        [RCIN_AILE_IDX] = TIMER1_MICROS_TO_TICKS(1000),     /* 1.0 ms by default */
        [RCIN_ELEV_IDX] = TIMER1_MICROS_TO_TICKS(1000),     /* 1.0 ms by default */
        [RCIN_RUDD_IDX] = TIMER1_MICROS_TO_TICKS(1000),     /* 1.0 ms by default */
        [RCIN_AUX1_IDX] = TIMER1_MICROS_TO_TICKS(1000),     /* 1.0 ms by default */
    },

    /* default failsafe control pulse of each RCIN channel */
    .rc_in_failsafe_ticks =
    {
        [RCIN_THRO_IDX] = TIMER1_MICROS_TO_TICKS(1000),     /* Shutdown engine */
        [RCIN_AILE_IDX] = TIMER1_MICROS_TO_TICKS(1500),     /* Set to neutral */
        [RCIN_ELEV_IDX] = TIMER1_MICROS_TO_TICKS(1500),     /* Set to neutral */
        [RCIN_RUDD_IDX] = TIMER1_MICROS_TO_TICKS(1500),     /* Set to neutral */
        [RCIN_AUX1_IDX] = TIMER1_MICROS_TO_TICKS(2000),     /* Return to home */
    },

    /* default roll PID tuning setting */
    .pid_aile_cfg =
    {
        .KP = 21.00,                                        /* KP */
        .KI = 7.14,                                         /* KI */
        .KD = 0.76,                                         /* KD */
        .scale = 1.0,                                       /* PID scale */
        .integral_max = 5.0,                                /* +- 5 degree */
        .output_max = TIMER1_MICROS_TO_TICKS(500),          /* +- 500 us */
    },

    /* default pitch PID tuning setting */
    .pid_elev_cfg =
    {
        .KP = 23.80,                                        /* KP */
        .KI = 10.23,                                        /* KI */
        .KD = 0.46,                                         /* KD */
        .scale = 1.0,                                       /* PID scale */
        .integral_max = 5.0,                                /* +- 5 degree */
        .output_max = TIMER1_MICROS_TO_TICKS(500),          /* +- 500 us */
    },

    /* default yaw PID tuning setting */
    .pid_rudd_cfg =
    {
        .KP = 14.0,                                         /* KP */
        .KI = 3.80,                                         /* KI */
        .KD = 0.5,                                          /* KD */
        .scale = 1.0,                                       /* PID scale */
        .integral_max = 5.0,                                /* +- 5 degree */
        .output_max = TIMER1_MICROS_TO_TICKS(500),          /* +- 500 us */
    },

    /* default bank turn (yaw to roll) PID tuning setting */
    .pid_bank_turn_cfg =
    {
        .KP = 0.8,                                          /* KP */
        .KI = 0.5,                                          /* KI */
        .KD = 0.005,                                        /* KD */
        .scale = 1.0,                                       /* PID scale */
        .integral_max = 5.0,                                /* +- 0 degree */
        .output_max = 30.0,                                 /* +- 30 degree */
    },

    .navigation =
    {
        .loiter_radius = 0.0,

        .current_wpt_idx = 0,
        .total_wpt = 0,
        .wpt = {
            [0] = {false, {0.0, 0.0}},
            [1] = {false, {0.0, 0.0}},
            [2] = {false, {0.0, 0.0}},
            [3] = {false, {0.0, 0.0}},
            [4] = {false, {0.0, 0.0}},
        },
    },

    .rom_crc16 = 0xFFFF,
};

/* Airplane runtime status */
static AIRPLANE_STATUS Airplane_Status =
{
    /* AHRS computation, will be initialized later */
    .ahrs_data = {0},

    /* PID computation, will be initialized later */
    .pid_aile_servo = {0},
    .pid_elev_servo = {0},
    .pid_rudd_servo = {0},
    .pid_band_turn = {0},

    .setpoint =
    {
        .roll_angle = 0.0,
        .pitch_angle = 0.0,
        .heading_angle = 0.0,
    },

    /* default RC channel input value */
    .rc_pulse_in =
    {
        [RCIN_THRO_IDX] = TIMER1_MICROS_TO_TICKS(0),
        [RCIN_AILE_IDX] = TIMER1_MICROS_TO_TICKS(0),
        [RCIN_ELEV_IDX] = TIMER1_MICROS_TO_TICKS(0),
        [RCIN_RUDD_IDX] = TIMER1_MICROS_TO_TICKS(0),
        [RCIN_AUX1_IDX] = TIMER1_MICROS_TO_TICKS(0)
    },

    /* default RC channel output value */
    .rc_pulse_out =
    {
        [RCIN_THRO_IDX] = TIMER1_MICROS_TO_TICKS(1000),
        [RCIN_AILE_IDX] = TIMER1_MICROS_TO_TICKS(1500),
        [RCIN_ELEV_IDX] = TIMER1_MICROS_TO_TICKS(1500),
        [RCIN_RUDD_IDX] = TIMER1_MICROS_TO_TICKS(1500),
    },

    .general =
    {
        .fly_mode = AIRPLANE_MANUAL_FLY,                    /* Manual fly by default */

        .imu_fail_cnt = 0,
        .ahrs_delay_cnt = 0,
        .rcin_cyc_cnt = 0,
        .rcout_cyc_cnt = 0,
        .delta_ctrl_time = 0,

        .mcu_vcc = 0.0,
    },

    .current_cruise_state = AIRPLANE_CRUISE_AWAYFROM_WPT,

    .heartbeat = 0,
};

/* GPS and navigation status */
static GPS_DATA Airplane_GPS;

#if AIRPLANE_STATUS_SNAPSHOT_EN
static AIRPLANE_STATUS Airplane_StatusSnapshot = {0};
#endif


/*
 *******************************************************************************
 * Public functions declaration
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Private functions declaration
 *******************************************************************************
 */

static int8_t Airplane_Init();
static void Airplane_FlyCtrl();
static float Airplane_BankToPitchCompensate(float roll_angle);
static int8_t Airplane_LoadConfig(AIRPLANE_CONFIG *p_config);
static int8_t Airplane_SaveConfig(AIRPLANE_CONFIG *p_config);
static int8_t Airplane_ClearConfig(AIRPLANE_CONFIG *p_config);
static void Airplane_RemoteCtrlCalibration();
static void Airplane_ConfigControl();
static uint8_t Airplane_GetDipSwOpt();
static void Airplane_UpdateAdcIO();
static int8_t Airplane_MixRC(int16_t *p_aile_mix_diff, int16_t *p_elev_mix_diff,
                             int16_t *p_rudd_mix_diff, AIRPLANE_TYPE wing_type);
static void Airplane_UpdatePidParam();
static AIRPLANE_FLY_MODE Airplane_ChkFlyMode(uint16_t *p_rc_in);
static float Airplane_CalAngleDiff(float current_angle, float target_angle,
                                   float max_angle, float min_angle);
static void Airplane_TxMessage(uint32_t delta_time);
static void Airplane_RxMessage();


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * setup - Initializing function of Arduino.
 *
 * @param   [none]
 * @return  [none]
 *
 */
void setup()
{
    Airplane_Init();
}

/**
 * loop - Arduino main loop function.
 *
 * @param   [none]
 * @return  [none]
 *
 */
void loop()
{
    Airplane_FlyCtrl();

    /* Blink twice per second for AIRPLANE_RETURN_TO_HOME mode. */
    if(Airplane_Status.general.fly_mode == AIRPLANE_RETURN_TO_HOME){
        LEDS_Lightning(LEDS_MASTER_IDX, 600, 100, 100);
    }
    /* Blink twice per 2 second for AIRPLANE_SELF_STABILIZE mode. */
    else if(Airplane_Status.general.fly_mode == AIRPLANE_SELF_STABILIZE){
        LEDS_Lightning(LEDS_MASTER_IDX, 1600, 100, 100);
    }
    /* Blink twice per 4 second for manual mode. */
    else{
        LEDS_Lightning(LEDS_MASTER_IDX, 3600, 100, 100);
    }
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

/**
 * Airplane_Init - Function to initialize airplane flight controller.
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t Airplane_Init()
{
    const char *p_ret_msg[] = {"OK", "Fail"};
    int8_t imu_error = 0;
    int8_t rom_error = 0;
    IMU_SENSOR_DATA imu_sensor_data = {0};
    uint8_t total_wpt;
    uint8_t current_wpt_idx;

    /* UART0 initialization */
    Uart0_Init(57600);
    Uart0_Println(PSTR("[Airplane] ONERC_LIB"));
    Uart0_Println(PSTR("[Airplane] FW date = %X"), AIRPLANE_FW_DATE);

    /* LEDs initialization */
    LEDS_Init();

    /* Timers initialization */
    Timers_Init();

    /* ADC pins initialization */
    ADC_Init();

    /* I2C initialization */
    I2C_Init(400000);

    /* Pin change function initialization */
    PC_Init();

#if UARTS_FUNCTION_EN
    /* Simulated UART initialization */
    UartS_Init(9600);
#endif

	/* MCU protocol initialization */
    MP_Init();

    /* Load airplane configuration from EEPROM */
    Uart0_Printf(PSTR("[ROM] Load: "));
    rom_error = Airplane_LoadConfig(&Airplane_Config);
    Uart0_Println(PSTR("%s (0x%hX)"),
                  rom_error ? p_ret_msg[1] : p_ret_msg[0],
                  Airplane_Config.rom_crc16);
    Uart0_Println(PSTR("[ROM] ID = %X, date = %X"),
                  Airplane_Config.config_ID,
                  Airplane_Config.config_date);

    /* Initial RC input channel */
    RCIN_SetNeutral(Airplane_Config.rc_in_neutral_ticks);
    RCIN_SetFailsafe(Airplane_Config.rc_in_failsafe_ticks);
    RCIN_SetDirection(Airplane_Config.rc_in_is_reversed);
    RCIN_SetMaxMinStick(Airplane_Config.rc_in_max_ticks, Airplane_Config.rc_in_min_ticks);
    RCIN_Init();

    /* Initial RC output channel */
    RCOUT_Init();

    /* Initial fail safe function */
    FailSafe_Init();

    /* Initial GPS hardware module and functions */
    GPS_Init(&Airplane_GPS);

    /* Setup first waypoint if it's activated */
    total_wpt = Airplane_Config.navigation.total_wpt;
    current_wpt_idx = Airplane_Config.navigation.current_wpt_idx;
    if(total_wpt > 0 && Airplane_Config.navigation.wpt[current_wpt_idx].is_actived == true){

        LEDS_PwrON(LEDS_MASTER_IDX);
        LEDS_PwrON(LEDS_SLAVE_IDX);
        Timer1_DelayMillis(500);
        LEDS_PwrOFF(LEDS_MASTER_IDX);
        LEDS_PwrOFF(LEDS_SLAVE_IDX);

        GPS_SetWpt(&Airplane_GPS, &Airplane_Config.navigation.wpt[current_wpt_idx].wpt_coord);

        Uart0_Printf(PSTR("[GPS] home: "));
        Uart0_Printf(PSTR("LAT_DD = "));
        Uart0_PrintFloat(Airplane_Config.navigation.wpt[current_wpt_idx].wpt_coord.LAT_DD, 4);
        Uart0_Printf(PSTR(", LONG_DD = "));
        Uart0_PrintFloat(Airplane_Config.navigation.wpt[current_wpt_idx].wpt_coord.LONG_DD, 4);
        Uart0_Println(PSTR(""));
    }

    Uart0_Println(PSTR("[GPS] loiter radius = %f meters"), AIRPLANE_GET_LOITER_RADIUS());

    /* Initial IMU sensors */
    LEDS_PwrON(LEDS_SLAVE_IDX);

    imu_error = IMU_Init();
    if(imu_error != 0){
        while(1);
    }
    else{
        /* Apply IMU bias setting directly if the ROM data is available */
        if(Airplane_Config.is_imu_calibrated == true)
            IMU_SetCalibratedBias(Airplane_Config.accel_bias, Airplane_Config.gyro_bias);
        /* Otherwise, perform calibration procedure */
        else
            IMU_DoCalibration(IMU_SENSOR_CAL_MODE);

        Uart0_Println(PSTR("[IMU] Accel Bias XYZ: %hd, %hd, %hd"),
                      Airplane_Config.accel_bias[0],
                      Airplane_Config.accel_bias[1],
                      Airplane_Config.accel_bias[2]);

        Uart0_Println(PSTR("[IMU] Gyro Bias XYZ: %hd, %hd, %hd"),
                      Airplane_Config.gyro_bias[0],
                      Airplane_Config.gyro_bias[1],
                      Airplane_Config.gyro_bias[2]);
    }

    LEDS_PwrOFF(LEDS_SLAVE_IDX);

    /* Initial AHRS function */
    IMU_Get6RawData(&imu_sensor_data);
    AHRS_Init(&(Airplane_Status.ahrs_data), imu_sensor_data.accel_raw);

    /* Initial PID data and parameters for ROLL */
    PID_Create(&Airplane_Status.pid_aile_servo);
    PID_SetTuning(&Airplane_Status.pid_aile_servo,
                  Airplane_Config.pid_aile_cfg.KP,
                  Airplane_Config.pid_aile_cfg.KI,
                  Airplane_Config.pid_aile_cfg.KD);
    PID_SetScaleFactor(&Airplane_Status.pid_aile_servo, Airplane_Config.pid_aile_cfg.scale);
    PID_SetIntegralMax(&Airplane_Status.pid_aile_servo, Airplane_Config.pid_aile_cfg.integral_max);
    PID_SetOutputMax(&Airplane_Status.pid_aile_servo, Airplane_Config.pid_aile_cfg.output_max);

    /* Initial PID data and parameters for PITCH */
    PID_Create(&Airplane_Status.pid_elev_servo);
    PID_SetTuning(&Airplane_Status.pid_elev_servo,
                  Airplane_Config.pid_elev_cfg.KP,
                  Airplane_Config.pid_elev_cfg.KI,
                  Airplane_Config.pid_elev_cfg.KD);
    PID_SetScaleFactor(&Airplane_Status.pid_elev_servo, Airplane_Config.pid_elev_cfg.scale);
    PID_SetIntegralMax(&Airplane_Status.pid_elev_servo, Airplane_Config.pid_elev_cfg.integral_max);
    PID_SetOutputMax(&Airplane_Status.pid_elev_servo, Airplane_Config.pid_elev_cfg.output_max);

    /* Initial PID data and parameters for YAW */
    PID_Create(&Airplane_Status.pid_rudd_servo);
    PID_SetTuning(&Airplane_Status.pid_rudd_servo,
                  Airplane_Config.pid_rudd_cfg.KP,
                  Airplane_Config.pid_rudd_cfg.KI,
                  Airplane_Config.pid_rudd_cfg.KD);
    PID_SetScaleFactor(&Airplane_Status.pid_rudd_servo, Airplane_Config.pid_rudd_cfg.scale);
    PID_SetIntegralMax(&Airplane_Status.pid_rudd_servo, Airplane_Config.pid_rudd_cfg.integral_max);
    PID_SetOutputMax(&Airplane_Status.pid_rudd_servo, Airplane_Config.pid_rudd_cfg.output_max);

    /* Initial PID data and parameters for bank turn */
    PID_Create(&Airplane_Status.pid_band_turn);
    PID_SetTuning(&Airplane_Status.pid_band_turn,
                  Airplane_Config.pid_bank_turn_cfg.KP,
                  Airplane_Config.pid_bank_turn_cfg.KI,
                  Airplane_Config.pid_bank_turn_cfg.KD);
    PID_SetScaleFactor(&Airplane_Status.pid_band_turn, Airplane_Config.pid_bank_turn_cfg.scale);
    PID_SetIntegralMax(&Airplane_Status.pid_band_turn, Airplane_Config.pid_bank_turn_cfg.integral_max);
    PID_SetOutputMax(&Airplane_Status.pid_band_turn, Airplane_Config.pid_bank_turn_cfg.output_max);

    Airplane_RemoteCtrlCalibration();

    /* Launch related initializing procedure and store new configuration to ROM if needed */
    Airplane_ConfigControl();

    return 0;
}

/**
 * Airplane_FlyCtrl - Airplane_FlyCtrl
 *
 * @param   [none]
 * @return  [none]
 *
 */
static void Airplane_FlyCtrl()
{
    static uint32_t prev_ctrl_update = Timer1_GetMicros();
    IMU_SENSOR_DATA imu_sensor_data = {0};
    AIRPLANE_NAVIGATION *p_nav_config;
    uint32_t current_ctrl_time;
    uint32_t delta_ctrl_time;
    int16_t rc_in_diff[RCIN_CH_TOTAL];
    uint8_t prev_wpt_idx;
    float roll_cosine;
    float pitch_pid_gain;
    float pitch_setpoint;
    float roll_angle_diff = 0.0;
    float pitch_angle_diff = 0.0;
    float heading_angle_diff = 0.0;
    float wpt_distance = 0;
    int16_t aile_pid_val = 0;
    int16_t elev_pid_val = 0;
    int16_t rudd_pid_val = 0;
    int16_t aile_out_diff = 0;
    int16_t elev_out_diff = 0;
    int16_t rudd_out_diff = 0;
    bool is_manual_aile;
    bool is_manual_elev;
    bool is_manual_rudd;
    bool is_nav_updated;

    /* Check time difference */
    current_ctrl_time = Timer1_GetMicros();
    delta_ctrl_time = current_ctrl_time - prev_ctrl_update;

    /* Update AHRS, PID and output PWM every 5 ms */
    if(delta_ctrl_time >= AIRPLANE_CTRL_LOOP_PERIOD){

        /* Read accelerometer and gyroscope raw data */
        if(IMU_Get6RawData(&imu_sensor_data) == 0){
            /* Update AHRS */
            AHRS_AttAngleUpdate(imu_sensor_data.accel_raw, imu_sensor_data.gyro_raw,
                                (uint16_t)delta_ctrl_time, &(Airplane_Status.ahrs_data));
        }
        else{
            Airplane_Status.general.imu_fail_cnt++;
        }

        /*
         * Update navigation status base on GPS report information,
         * this function may occupies 1 ms runtime per call.
         */
        is_nav_updated = false;

        if(GPS_UpdateNMEA(&Airplane_GPS) != GPS_RX_NMEA_TYPE_UNKNOWN){
            if(GPS_UpdateNav(&Airplane_GPS) == 0){
                is_nav_updated = true;
            }

            LEDS_PwrON(LEDS_SLAVE_IDX);
        }
        else{
            LEDS_PwrOFF(LEDS_SLAVE_IDX);
        }

        /* Read latest RC input value, range 0 or 1000 ~ 2000 us */
        Airplane_Status.general.rcin_cyc_cnt = RCIN_ReadChannels(Airplane_Status.rc_pulse_in);
        RCIN_GetChannelsDiff(Airplane_Status.rc_pulse_in, rc_in_diff);

        /* Check current fly mode according the input PWM width on AUX channel */
        Airplane_Status.general.fly_mode = Airplane_ChkFlyMode(Airplane_Status.rc_pulse_in);

        /* Reset PID for manual mode */
        if(Airplane_Status.general.fly_mode == AIRPLANE_MANUAL_FLY){

            aile_pid_val = 0;
            elev_pid_val = 0;
            rudd_pid_val = 0;

            Airplane_Status.setpoint.roll_angle = 0.0;
            Airplane_Status.setpoint.pitch_angle = 0.0;
            Airplane_Status.setpoint.heading_angle = Airplane_Status.ahrs_data.ned_att.heading_angle;
            Airplane_Status.current_cruise_state = AIRPLANE_CRUISE_FORWARDTO_WPT;

            PID_Reset(&Airplane_Status.pid_aile_servo);
            PID_Reset(&Airplane_Status.pid_elev_servo);
            PID_Reset(&Airplane_Status.pid_rudd_servo);
            PID_Reset(&Airplane_Status.pid_band_turn);
        }
        /* Update PID for auto level control or heading control */
        else{

            is_manual_aile = (abs(rc_in_diff[RCIN_AILE_IDX]) >= TIMER1_MICROS_TO_TICKS(20)) ? true : false;
            is_manual_elev = (abs(rc_in_diff[RCIN_ELEV_IDX]) >= TIMER1_MICROS_TO_TICKS(20)) ? true : false;
            is_manual_rudd = (abs(rc_in_diff[RCIN_RUDD_IDX]) >= TIMER1_MICROS_TO_TICKS(20)) ? true : false;

            /* Return to home */
            if(Airplane_Status.general.fly_mode == AIRPLANE_RETURN_TO_HOME){

                if(is_nav_updated == true){

                    GPS_GetWptDistance(&Airplane_GPS, &wpt_distance);

                    /* Loitering ...
                     * we just passed the home point, so we force the airplane keeps current
                     * heading angle and keeps forward and away from current way point until
                     * the current distance to home point is larger than loiter_radius, and
                     * then we return to home point again.
                     */
                    if(Airplane_Status.current_cruise_state == AIRPLANE_CRUISE_AWAYFROM_WPT){
                        if(wpt_distance > Airplane_Config.navigation.loiter_radius)
                            Airplane_Status.current_cruise_state = AIRPLANE_CRUISE_FORWARDTO_WPT;
                    }
                    /* Forward to way point */
                    else{
                        /*
                         * The airplane is very closed to current way point, so it's time to switch to
                         * next way point if it's available, otherwise we force the airplane loiters around
                         * current way point.
                         */
                        if(wpt_distance <= AIRPLANE_WPT_ARRIVE_RADIUS){
                            p_nav_config = &Airplane_Config.navigation;

                            prev_wpt_idx = p_nav_config->current_wpt_idx;
                            p_nav_config->current_wpt_idx++;
                            if(p_nav_config->current_wpt_idx >= p_nav_config->total_wpt)
                                p_nav_config->current_wpt_idx = 0;

                            /* Update new waypoint if it's available */
                            if(prev_wpt_idx != p_nav_config->current_wpt_idx
                               && p_nav_config->wpt[p_nav_config->current_wpt_idx].is_actived){
                                GPS_SetWpt(&Airplane_GPS, &(p_nav_config->wpt[p_nav_config->current_wpt_idx].wpt_coord));
                            }
                            /* Otherwise, enter loitering mode */
                            else{
                                Airplane_Status.current_cruise_state = AIRPLANE_CRUISE_AWAYFROM_WPT;
                            }
                        }
                        /* We are still faraway to current waypoint, keeping adjust heading angle */
                        else{
                            Airplane_Status.setpoint.heading_angle = Airplane_Status.ahrs_data.ned_att.heading_angle
                                                                   + GPS_GetWptRelativeBearing(&Airplane_GPS);
                        }
                    }
                }
            }
            /* Stabilize mode */
            else{
                /* Do nothing */
                Airplane_Status.current_cruise_state = AIRPLANE_CRUISE_AWAYFROM_WPT;
            }

            /* Update heading setpoint when the airplane pilot trying to control airplane roll or yaw manually */
            if(is_manual_aile == true || is_manual_rudd == true){
                Airplane_Status.setpoint.heading_angle = Airplane_Status.ahrs_data.ned_att.heading_angle;
            }

            /* Heading (bank turn), convert expected NED heading -> roll angle -> ailerons servo */
            heading_angle_diff = Airplane_CalAngleDiff(Airplane_Status.ahrs_data.ned_att.heading_angle,
                                                       Airplane_Status.setpoint.heading_angle,
                                                       180.0, -180.0);

            Airplane_Status.setpoint.roll_angle = (int16_t)PID_Update(&Airplane_Status.pid_band_turn,
                                                                      -heading_angle_diff,
                                                                      (uint16_t)delta_ctrl_time,
                                                                      !(is_manual_aile || is_manual_rudd));

            /*
             * Compensate pitch setpoint and elevator PID scale according to current roll angle.
             * The airplane lose cos(bank_angle) lift during bank turn, and the torque of
             * elevator is reduced too, so we need to increase the pitch angle and increase the
             * elevator PID scale during the turn to prevent the airplane loses altitude.
             */
            roll_cosine = cos(MATH_DEG2RAD(Airplane_Status.ahrs_data.ned_att.roll_angle));

            /* Increase NED pitch angle */
            pitch_setpoint = (1.0 - roll_cosine) * AIRPLANE_BANK_TURN_PITCH_GAIN;
            Airplane_Status.setpoint.pitch_angle = pitch_setpoint;

            /* Increase elevator control gain (the divisor should not equal to 0) */
            pitch_pid_gain = fabs(roll_cosine);
            if(pitch_pid_gain != 0.0)
                pitch_pid_gain = 1.0 / roll_cosine;
            else
                pitch_pid_gain = 1.0;

            pitch_pid_gain = pitch_pid_gain * Airplane_Config.pid_elev_cfg.scale;
            PID_SetScaleFactor(&Airplane_Status.pid_elev_servo, pitch_pid_gain);

            /* Calculate the angle difference between current attitude and expected attitude. */
            roll_angle_diff = Airplane_CalAngleDiff(Airplane_Status.ahrs_data.ned_att.roll_angle,
                                                    Airplane_Status.setpoint.roll_angle,
                                                    180.0, -180.0);

            pitch_angle_diff = Airplane_CalAngleDiff(Airplane_Status.ahrs_data.ned_att.pitch_angle,
                                                     Airplane_Status.setpoint.pitch_angle,
                                                     90.0, -90.0);

            /* Update ailerons and elevator and rudder servo control PID */
            aile_pid_val = (int16_t)PID_Update(&Airplane_Status.pid_aile_servo,
                                               roll_angle_diff,
                                               (uint16_t)delta_ctrl_time,
                                               !(is_manual_aile || is_manual_rudd));
            elev_pid_val = (int16_t)PID_Update(&Airplane_Status.pid_elev_servo,
                                               pitch_angle_diff,
                                               (uint16_t)delta_ctrl_time,
                                               !is_manual_elev);
            rudd_pid_val = (int16_t)PID_Update(&Airplane_Status.pid_rudd_servo,
                                               0.0,
                                               (uint16_t)delta_ctrl_time,
                                               !is_manual_rudd);

        }

        /* Decide output control value */
        aile_out_diff = aile_pid_val + rc_in_diff[RCIN_AILE_IDX];
        elev_out_diff = elev_pid_val + rc_in_diff[RCIN_ELEV_IDX];
        rudd_out_diff = rudd_pid_val + rc_in_diff[RCIN_RUDD_IDX];

        /* RC output mixer */
        Airplane_MixRC(&aile_out_diff, &elev_out_diff, &rudd_out_diff, Airplane_Config.model_type);

        /* Decide width of output control pulse for AILE, ELEV and RUDD */
        Airplane_Status.rc_pulse_out[RCOUT_AILE_IDX] = Airplane_Config.rc_in_neutral_ticks[RCOUT_AILE_IDX] + aile_out_diff;
        Airplane_Status.rc_pulse_out[RCOUT_ELEV_IDX] = Airplane_Config.rc_in_neutral_ticks[RCOUT_ELEV_IDX] + elev_out_diff;
        Airplane_Status.rc_pulse_out[RCOUT_RUDD_IDX] = Airplane_Config.rc_in_neutral_ticks[RCOUT_RUDD_IDX] + rudd_out_diff;

        /* Correct AILE, ELEV and RUDD pulse output range to 1000 ~ 2000 us */
        Airplane_Status.rc_pulse_out[RCOUT_AILE_IDX] = constrain(Airplane_Status.rc_pulse_out[RCOUT_AILE_IDX],
                                                                 TIMER1_MICROS_TO_TICKS(1000),
                                                                 TIMER1_MICROS_TO_TICKS(2000));
        Airplane_Status.rc_pulse_out[RCOUT_ELEV_IDX] = constrain(Airplane_Status.rc_pulse_out[RCOUT_ELEV_IDX],
                                                                 TIMER1_MICROS_TO_TICKS(1000),
                                                                 TIMER1_MICROS_TO_TICKS(2000));
        Airplane_Status.rc_pulse_out[RCOUT_RUDD_IDX] = constrain(Airplane_Status.rc_pulse_out[RCOUT_RUDD_IDX],
                                                                 TIMER1_MICROS_TO_TICKS(1000),
                                                                 TIMER1_MICROS_TO_TICKS(2000));

        /* Output THRO pulse directly */
        Airplane_Status.rc_pulse_out[RCOUT_THRO_IDX] = constrain(Airplane_Status.rc_pulse_in[RCIN_THRO_IDX],
                                                                 TIMER1_MICROS_TO_TICKS(1000),
                                                                 TIMER1_MICROS_TO_TICKS(2000));

        /* Update ADC based data */
        Airplane_UpdateAdcIO();

        /* Update output PPM/PWM pulse width */
        RCOUT_SetServoPWM(Airplane_Status.rc_pulse_out, RCOUT_CH_TOTAL);
        Airplane_Status.general.rcout_cyc_cnt = RCOUT_GetCycUpdateCnt();

        Airplane_Status.general.delta_ctrl_time = delta_ctrl_time;
        Airplane_Status.heartbeat = Timer1_GetMillis();

        if(delta_ctrl_time > AIRPLANE_CTRL_LOOP_DELAY_THR){
            Airplane_Status.general.ahrs_delay_cnt++;
        }

        prev_ctrl_update = current_ctrl_time;

        /* Receive protocol message */
        Airplane_RxMessage();

        /* Transmit protocol message */
        Airplane_TxMessage(delta_ctrl_time);
    }
}

/**
 * Airplane_BankToPitchCompensate - Function to calculate pitch compensation angle
 *                                  for bank turn.
 *
 * @param   [in]        roll_angle      Current roll angle, -180 ~ 180.
 *
 * @return  [float]     Expected pitch compensation angle.
 *
 */
static float Airplane_BankToPitchCompensate(float roll_angle)
{
    float pitch_angle;

    pitch_angle = (1.0 - cos(MATH_DEG2RAD(roll_angle))) * AIRPLANE_BANK_TURN_PITCH_GAIN;
    pitch_angle = constrain(pitch_angle, -AIRPLANE_BANK_TURN_MAX_PITCH, AIRPLANE_BANK_TURN_MAX_PITCH);

    return pitch_angle;
}

/**
 * Airplane_LoadConfig - Function to load airplane configuration from ROM.

 * @param   [in]        *p_config       The airplane configuration buffer for
 *                                      storing ROM data.
 *                                      The airplane configuration buffer will
 *                                      only be updated if the loaded ROM is valid.
 *
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t Airplane_LoadConfig(AIRPLANE_CONFIG *p_config)
{
    AIRPLANE_CONFIG config_tmp;
    uint16_t rom_crc;
    int8_t ret_val;

    ret_val = -1;

    ROM_ReadBytes(AIRPLANE_CFG_ROM_ADDR, (uint8_t *)&config_tmp, sizeof(config_tmp));

    rom_crc = CRC_Calculate((uint8_t *)&config_tmp,
                            (sizeof(config_tmp) - sizeof(config_tmp.rom_crc16)));

    if(config_tmp.rom_crc16 == rom_crc && config_tmp.config_ID == AIRPLANE_CONFIG_ID){
        memcpy((void *)p_config, (void *)&config_tmp, sizeof(AIRPLANE_CONFIG));

        ret_val = 0;
    }

    return ret_val;
}

/**
 * Airplane_SaveConfig - Function to save airplane configuration to ROM.

 * @param   [in]        *p_config       The airplane configuration buffer which
 *                                      containing the data that want to be saved.
 *
 *                                      The CRC16 of airplane configuration will be
 *                                      updated automatically before we store the
 *                                      data to ROM.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t Airplane_SaveConfig(AIRPLANE_CONFIG *p_config)
{
    uint16_t rom_crc;
    int8_t ret_val;

    /* Calculate data CRC */
    rom_crc = CRC_Calculate((uint8_t *)p_config,
                            (sizeof(AIRPLANE_CONFIG) - sizeof(p_config->rom_crc16)));

    /* Update data CRC */
    p_config->rom_crc16 = rom_crc;

    /* Save to ROM */
    ROM_UpdateBytes(AIRPLANE_CFG_ROM_ADDR, (uint8_t *)p_config, sizeof(AIRPLANE_CONFIG));

    /* Read after write, make sure the data is saved correctly */
    ret_val = Airplane_LoadConfig(p_config);

    return ret_val;
}

/**
 * Airplane_ClearConfig - Function to clear airplane configuration stored in ROM.

 * @param   [in]        *p_config       The airplane configuration buffer which
 *                                      containing the data that want to be saved.
 *
 *                                      The CRC16 of airplane configuration will be
 *                                      updated automatically before we store the
 *                                      data to ROM.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t Airplane_ClearConfig(AIRPLANE_CONFIG *p_config)
{
    int8_t ret_val;

    /* Destroy CRC */
    p_config->rom_crc16 += 0x1234;

    /* Save to ROM */
    ret_val = ROM_UpdateBytes(AIRPLANE_CFG_ROM_ADDR,
                              (uint8_t *)p_config,
                              sizeof(AIRPLANE_CONFIG));

    return ret_val;
}

/**
 * Airplane_RemoteCtrlCalibration - Airplane_RemoteCtrlCalibration
 *
 * @param   [none]
 * @return  [none]
 *
 */
static void Airplane_RemoteCtrlCalibration()
{
    uint8_t dip_current_position;
    uint8_t dip_tmp_position;
    uint8_t debounce_cnt;
    uint8_t cali_mode;
    uint8_t rcin_channel;
    uint8_t blink_cnt;
    uint16_t rcin_sample_cnt;
    uint16_t rcin_new[RCIN_CH_TOTAL];
    uint16_t rcin_max[RCIN_CH_TOTAL];
    uint16_t rcin_min[RCIN_CH_TOTAL];
    uint32_t rcin_smooth[RCIN_CH_TOTAL];
    bool is_new_command;

    dip_current_position = Airplane_GetDipSwOpt();

    if(dip_current_position == AIRPLANE_DIP_RC_CALI_ACTIVE){

        LEDS_PwrON(LEDS_SLAVE_IDX);

        cali_mode = AIRPLANE_RC_CALI_NONE;
        is_new_command = false;

        while(1){

            /* Get current dip switch position with debounce filter */
            debounce_cnt = 0;
            while(debounce_cnt < 10){
                dip_tmp_position = Airplane_GetDipSwOpt();

                if(dip_tmp_position != dip_current_position){
                    dip_current_position = dip_tmp_position;
                    debounce_cnt = 0;
                }
                else{
                    debounce_cnt++;
                }

                Timer1_DelayMillis(1);
            }

            /* Blink LED slowly when there is no DIP command request */
            if(dip_current_position == AIRPLANE_DIP_RC_CALI_ACTIVE && is_new_command == false){
                LEDS_Blink(LEDS_MASTER_IDX, 50, 950);

            }
            /* Received DIP command request */
            else if(dip_current_position == AIRPLANE_DIP_RC_CALI_START){

                LEDS_Blink(LEDS_MASTER_IDX, 50, 60);

                /* Initialize rc input variables */
                if(is_new_command == false){

                    memset((void *)rcin_smooth, 0, sizeof(rcin_smooth));

                    for(rcin_channel = 0; rcin_channel < RCIN_CH_TOTAL; rcin_channel++){
                        rcin_max[rcin_channel] = 0;
                        rcin_min[rcin_channel] = 0xFFFF;
                    }

                    is_new_command = true;
                    cali_mode++;
                }
                else{
                    /* Keep update TX stick MAX/MIN pulse ticks for RC stick range calibration mode */
                    if(cali_mode == AIRPLANE_RC_CALI_RANGE){
                        RCIN_ReadChannels(rcin_new);

                        for(rcin_channel = 0; rcin_channel < RCIN_CH_TOTAL; rcin_channel++){
                            if(RCIN_IS_PULSE_VALID(rcin_new[rcin_channel]) == true){
                                rcin_max[rcin_channel] = MATH_MAX(rcin_new[rcin_channel], rcin_max[rcin_channel]);
                                rcin_min[rcin_channel] = MATH_MIN(rcin_new[rcin_channel], rcin_min[rcin_channel]);
                            }
                        }
                    }
                }

            }
            /* Done current command request and apply new setting */
            else if(dip_current_position == AIRPLANE_DIP_RC_CALI_ACTIVE && is_new_command == true){

                LEDS_PwrOFF(LEDS_MASTER_IDX);

                /* Collect RC input pulse samples */
                rcin_sample_cnt = 0;
                while(rcin_sample_cnt < AIRPLANE_RC_CALI_SMOOTH_CNT){

                    RCIN_ReadChannels(rcin_new);

                    for(rcin_channel = 0; rcin_channel < RCIN_CH_TOTAL; rcin_channel++){
                        rcin_smooth[rcin_channel] += rcin_new[rcin_channel];
                    }

                    Timer1_DelayMillis(AIRPLANE_RC_CALI_SMOOTH_PERIOD);

                    rcin_sample_cnt++;
                }

                /* Calculate average of collected input pulse samples  */
                for(rcin_channel = 0; rcin_channel < RCIN_CH_TOTAL; rcin_channel++)
                    rcin_smooth[rcin_channel] /= AIRPLANE_RC_CALI_SMOOTH_CNT;

                /* Apply TX stick neutral configuration */
                if(cali_mode == AIRPLANE_RC_CALI_NEUTRAL){
                    for(rcin_channel = 0; rcin_channel < RCIN_CH_TOTAL; rcin_channel++){
                        Airplane_Config.rc_in_neutral_ticks[rcin_channel] = (uint16_t)rcin_smooth[rcin_channel];
                    }
                }
                /* Apply TX stick MAX/MIN range configuration */
                else if(cali_mode == AIRPLANE_RC_CALI_RANGE){
                    for(rcin_channel = 0; rcin_channel < RCIN_CH_TOTAL; rcin_channel++){
                        Airplane_Config.rc_in_max_ticks[rcin_channel] = rcin_max[rcin_channel];
                        Airplane_Config.rc_in_min_ticks[rcin_channel] = rcin_min[rcin_channel];
                    }
                }
                /* Apply TX radio failsafe configuration */
                else if(cali_mode == AIRPLANE_RC_CALI_FAILSAFE){
                    for(rcin_channel = 0; rcin_channel < RCIN_CH_TOTAL; rcin_channel++){
                        Airplane_Config.rc_in_failsafe_ticks[rcin_channel] = (uint16_t)rcin_smooth[rcin_channel];
                    }
                }

                /* Blink LED to indicate current status */
                for(blink_cnt = 0; blink_cnt < cali_mode; blink_cnt++){
                    LEDS_PwrOFF(LEDS_MASTER_IDX);
                    Timer1_DelayMillis(100);
                    LEDS_PwrON(LEDS_MASTER_IDX);
                    Timer1_DelayMillis(500);
                    LEDS_PwrOFF(LEDS_MASTER_IDX);
                    Timer1_DelayMillis(500);
                }

                /* Save current configuration to ROM after the user completes the RC calibration procedure */
                if(cali_mode + 1 == AIRPLANE_RC_CALI_FINISH){
                    Airplane_SaveConfig(&Airplane_Config);
                    FailSafe_Reboot();
                }

                /* We done current command request */
                is_new_command = false;
            }
        }

        LEDS_PwrOFF(LEDS_SLAVE_IDX);
    }
}

/**
 * Airplane_ConfigControl - Airplane_ConfigControl
 *
 * @param   [none]
 * @return  [none]
 *
 */
static void Airplane_ConfigControl()
{
    GPS_COORD_POINT home_point;
    uint8_t gps_sample_cnt;
    uint32_t detect_start;
    uint32_t led_time;
    uint32_t led_period;
    bool is_led_on;

    detect_start = Timer1_GetMillis();

    while((Timer1_GetMillis() - detect_start) < AIRPLANE_CHK_CFG_MODE_TIMEOUT){

        /* Check current RX pulse width */
        RCIN_ReadChannels(Airplane_Status.rc_pulse_in);

        /*
        * Activate IMU calibration procedure when current
        * TX elevator stick is in bottom position
        */
        if(Airplane_Status.rc_pulse_in[RCIN_ELEV_IDX] >= TIMER1_MICROS_TO_TICKS(900)
           && Airplane_Status.rc_pulse_in[RCIN_ELEV_IDX] <= TIMER1_MICROS_TO_TICKS(1200)){

            LEDS_PwrON(LEDS_MASTER_IDX);

            do{
                /* Check current RX pulse width */
                RCIN_ReadChannels(Airplane_Status.rc_pulse_in);

                /* Clear configuration */
                if(Airplane_Status.rc_pulse_in[RCIN_AILE_IDX] >= TIMER1_MICROS_TO_TICKS(1800)
                   && Airplane_Status.rc_pulse_in[RCIN_AILE_IDX] <= TIMER1_MICROS_TO_TICKS(2100)){

                    Airplane_ClearConfig(&Airplane_Config);

                    break;
                }
                /* Do IMU sensors calibration and save the bias to ROM */
                else if(Airplane_Status.rc_pulse_in[RCIN_AILE_IDX] >= TIMER1_MICROS_TO_TICKS(900)
                        && Airplane_Status.rc_pulse_in[RCIN_AILE_IDX] <= TIMER1_MICROS_TO_TICKS(1200)){

                    IMU_DoCalibration(IMU_SENSOR_CAL_RUNTIME);
                    IMU_GetCalibratedBias(Airplane_Config.accel_bias, Airplane_Config.gyro_bias);

                    Airplane_Config.is_imu_calibrated = true;
                    Airplane_SaveConfig(&Airplane_Config);

                    break;
                }

                LEDS_Blink(LEDS_SLAVE_IDX, 50, 950);

            }while(1);

            LEDS_PwrOFF(LEDS_MASTER_IDX);

            FailSafe_Reboot();
        }
        /*
        * Activate GPS homepoint setup procedure when TX elevator stick is in top position
        */
        else if(Airplane_Status.rc_pulse_in[RCIN_ELEV_IDX] >= TIMER1_MICROS_TO_TICKS(1800)
                && Airplane_Status.rc_pulse_in[RCIN_ELEV_IDX] <= TIMER1_MICROS_TO_TICKS(2100)){

            gps_sample_cnt = 0;

            is_led_on = true;
            led_time = Timer1_GetMillis();
            led_period = 1000;

            LEDS_PwrOFF(LEDS_MASTER_IDX);
            LEDS_PwrOFF(LEDS_SLAVE_IDX);

            /* Collecting current position samples */
            while(gps_sample_cnt < AIRPLANE_GPS_HOME_SAMPLE_CNT){

                /* We get current position from GPGGA frame */
                if(GPS_UpdateNMEA(&Airplane_GPS) == GPS_RX_NMEA_TYPE_GGA){

                    led_period = 100;

                    if(gps_sample_cnt == 0){
                        home_point.LAT_DD = Airplane_GPS.nmea.gpgga.coord.LAT_DD;
                        home_point.LONG_DD = Airplane_GPS.nmea.gpgga.coord.LONG_DD;
                    }
                    else{
                        home_point.LAT_DD += Airplane_GPS.nmea.gpgga.coord.LAT_DD;
                        home_point.LONG_DD += Airplane_GPS.nmea.gpgga.coord.LONG_DD;
                    }

                    gps_sample_cnt++;
                }

                /* Flash LED */
                if(Timer1_GetMillis() - led_time > led_period){
                    if(is_led_on == true){
                        LEDS_PwrOFF(LEDS_MASTER_IDX);
                        LEDS_PwrOFF(LEDS_SLAVE_IDX);
                    }
                    else{
                        LEDS_PwrON(LEDS_MASTER_IDX);
                        LEDS_PwrON(LEDS_SLAVE_IDX);
                    }

                    is_led_on ^= true;
                    led_time = Timer1_GetMillis();
                }

            }

            LEDS_PwrON(LEDS_MASTER_IDX);
            LEDS_PwrON(LEDS_SLAVE_IDX);

            /* Calculate current position and save to ROM */
            home_point.LAT_DD /= (float)AIRPLANE_GPS_HOME_SAMPLE_CNT;
            home_point.LONG_DD /= (float)AIRPLANE_GPS_HOME_SAMPLE_CNT;

            Airplane_Config.navigation.current_wpt_idx = 0;
            Airplane_Config.navigation.total_wpt = 1;
            Airplane_Config.navigation.wpt[0].is_actived = true;
            Airplane_Config.navigation.wpt[0].wpt_coord.LAT_DD = home_point.LAT_DD;
            Airplane_Config.navigation.wpt[0].wpt_coord.LONG_DD = home_point.LONG_DD;

            Airplane_SaveConfig(&Airplane_Config);

            /* We done the home position setup procedure, reboot now */
            LEDS_PwrOFF(LEDS_MASTER_IDX);
            LEDS_PwrOFF(LEDS_SLAVE_IDX);
            FailSafe_Reboot();
        }
    }
}

/**
 * Airplane_GetDipSwOpt - Function to read DIP setting via ADC.
 *
 * Vcc -- 04.7K Ohm -- Dip 1 --|-- ADC --4.7K ohm -- Gnd
 *     |- 10.0K Ohm -- Dip 2 --|
 *
 * @param   [none]
 * @return  [uint8_t]   Current DIP setting.
 * @retval  [0]         Dip 1 OFF, Dip 2 OFF.
 *          [1]         Dip 1 ON, Dip 2 OFF.
 *          [2]         Dip 1 OFF, Dip 2 ON.
 *          [3]         Dip 1 ON, Dip 2 ON.
 *
 */
static uint8_t Airplane_GetDipSwOpt()
{
    uint16_t dip_adc;
    uint8_t dip_opt;

    dip_adc = ADC_Read(AIRPLANE_MODE_DIPS_CH);

    /* ADC value around 609 */
    if(dip_adc > 560){
        dip_opt = 3;
    }
    /* ADC value around 512 */
    else if(dip_adc > 419){
        dip_opt = 2;
    }
    /* ADC value around 326 */
    else if(dip_adc > 163){
        dip_opt = 1;
    }
    /* ADC value around 0 */
    else{
        dip_opt = 0;
    }

    return dip_opt;
}

/**
 * Airplane_UpdateAdcIO - Function to update ADC IO value sequentially.
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
static void Airplane_UpdateAdcIO()
{
    static uint8_t adc_idx = 0;

    switch(adc_idx){

        /* Read PID tuning setting from according to external potentiometer */
        case 0:

#if AIRPLANE_PID_POT_EN
            Airplane_UpdatePidParam();
#endif
            break;

        /* Home position loiter radius (meters) */
        case 1:

            /* 20.0 ~ 224.6 meters */
            Airplane_Config.navigation.loiter_radius = AIRPLANE_GET_LOITER_RADIUS();

            break;

        /*
         * We compare the voltage difference between bandgap (1.1v) and AVCC
         * for getting current system voltage. The 1.1v needs to be stabilized by
         * apply some delay before we start the ADC conversation, so we change the
         * MUX setting to 1.1v first, and start the ADC conversation in second round.
         *
         * This function will be called per 5ms, means there is 5ms delay between
         * ADC_SetMuxTo1V1() and ADC_ReadSysVoltage(), should be enough for stabilizing
         * 1v1 voltage.
         */
        case 2:
            ADC_SetMuxTo1V1();

            break;

        case 3:
            Airplane_Status.general.mcu_vcc = ADC_ReadSysVoltage();

            break;

        default:
            break;
    }

    adc_idx++;
    if(adc_idx > 3)
        adc_idx = 0;
}

/**
 * Airplane_MixRC - Function to mix RC channel output.
 *
 * @param   [in/out]     *p_aile_mix_diff       Input original AILE servo pulse difference value,
 *                                              then this function output mixed AILE control pulse value.
 *                                              the input value range should be +- N ticks.

 * @param   [in/out]     *p_elev_mix_diff       Input original ELEV servo pulse difference value,
 *                                              then this function output mixed AILE control pulse value.
 *                                              the input value range should be +- N ticks.

 * @param   [in/out]     *p_rudd_mix_diff       Input original RUDD servo pulse difference value,
 *                                              then this function output mixed AILE control pulse value.
 *                                              the input value range should be +- N ticks.

 * @param   [input]     wing_type               Type of airplane.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t Airplane_MixRC(int16_t *p_aile_mix_diff, int16_t *p_elev_mix_diff,
                             int16_t *p_rudd_mix_diff, AIRPLANE_TYPE wing_type)
{
    int16_t aile_tmp_diff;
    int16_t elev_tmp_diff;
    int16_t rudd_tmp_diff;

    if(p_aile_mix_diff == NULL || p_elev_mix_diff == NULL || p_rudd_mix_diff == NULL)
        return -1;

    aile_tmp_diff = *p_aile_mix_diff;
    elev_tmp_diff = *p_elev_mix_diff;
    rudd_tmp_diff = *p_rudd_mix_diff;

    switch(wing_type){
        case AIRPLANE_DELTA:

            aile_tmp_diff *= 0.5;
            elev_tmp_diff *= 0.5;

            *p_aile_mix_diff = aile_tmp_diff + elev_tmp_diff;
            *p_elev_mix_diff = aile_tmp_diff - elev_tmp_diff;
            *p_rudd_mix_diff = rudd_tmp_diff;

            break;

        case AIRPLANE_VTAIL:

            elev_tmp_diff *= 0.5;
            rudd_tmp_diff *= 0.5;

            *p_aile_mix_diff = aile_tmp_diff;
            *p_elev_mix_diff = rudd_tmp_diff + elev_tmp_diff;
            *p_rudd_mix_diff = rudd_tmp_diff - elev_tmp_diff;

            break;

        case AIRPLANE_NORMAL:
        default:

            *p_aile_mix_diff = aile_tmp_diff;
            *p_elev_mix_diff = elev_tmp_diff;
            *p_rudd_mix_diff = rudd_tmp_diff;

            break;
    }

    return 0;
}

/**
 * Airplane_UpdatePidParam - Function to update PID parameters
 *                           according to on-board potentiometers.
 *
 * @param   [none]
 * @return  [none]
 *
 */
static void Airplane_UpdatePidParam()
{
    static uint8_t param_idx = 0;
    float KP, KI, KD;
    float roll_scale, pitch_scale, yaw_scale;

#if AIRPLANE_PID_POT_TYPE == AIRPLANE_PID_POT_COMMON

    /* Read KP, KI and KD setting from potentiometers */
    KP = ADC_Read(AIRPLANE_KP_CH) * 0.10;
    KI = ADC_Read(AIRPLANE_KI_CH) * 0.1;
    KD = ADC_Read(AIRPLANE_KD_CH) * 0.001;

    Airplane_Config.pid_aile_cfg.KP = KP;
    Airplane_Config.pid_aile_cfg.KI = KI;
    Airplane_Config.pid_aile_cfg.KD = KD;

    Airplane_Config.pid_elev_cfg.KP = KP;
    Airplane_Config.pid_elev_cfg.KI = KI;
    Airplane_Config.pid_elev_cfg.KD = KD;

    Airplane_Config.pid_rudd_cfg.KP = KP;
    Airplane_Config.pid_rudd_cfg.KI = KI;
    Airplane_Config.pid_rudd_cfg.KD = KD;

    /* Apply to roll stabilizing PID controller */
    PID_SetTuning(&Airplane_Status.pid_aile_servo,
                  Airplane_Config.pid_aile_cfg.KP,
                  Airplane_Config.pid_aile_cfg.KI,
                  Airplane_Config.pid_aile_cfg.KD);

    /* Apply to pitch stabilizing PID controller */
    PID_SetTuning(&Airplane_Status.pid_elev_servo,
                  Airplane_Config.pid_elev_cfg.KP,
                  Airplane_Config.pid_elev_cfg.KI,
                  Airplane_Config.pid_elev_cfg.KD);

    /* Apply to yaw stabilizing PID controller */
    PID_SetTuning(&Airplane_Status.pid_rudd_servo,
                  Airplane_Config.pid_rudd_cfg.KP,
                  Airplane_Config.pid_rudd_cfg.KI,
                  Airplane_Config.pid_rudd_cfg.KD);

#elif AIRPLANE_PID_POT_TYPE == AIRPLANE_PID_POT_SCALE

    /* Read PID scale setting from potentiometers (range 1.5~0.5, -0.5~-1.5) */
    switch(param_idx){
        case 0:

            roll_scale = ((ADC_Read(AIRPLANE_ROLL_SCALE_CH) - 512.0) * 0.001953125);
            roll_scale = (roll_scale > 0) ? (roll_scale + 0.5) : (roll_scale - 0.5);
            Airplane_Config.pid_aile_cfg.scale = roll_scale;
            PID_SetScaleFactor(&Airplane_Status.pid_aile_servo,
                               Airplane_Config.pid_aile_cfg.scale);

            break;

        case 1:

            pitch_scale = ((ADC_Read(AIRPLANE_PITCH_SCALE_CH) - 512.0) * 0.001953125);
            pitch_scale = (pitch_scale > 0) ? (pitch_scale + 0.5) : (pitch_scale - 0.5);
            Airplane_Config.pid_elev_cfg.scale = pitch_scale;
            PID_SetScaleFactor(&Airplane_Status.pid_elev_servo,
                               Airplane_Config.pid_elev_cfg.scale);
            break;

        case 2:

            yaw_scale = ((ADC_Read(AIRPLANE_YAW_SCALE_CH) - 512.0) * 0.001953125);
            yaw_scale = (yaw_scale > 0) ? (yaw_scale + 0.5) : (yaw_scale - 0.5);
            Airplane_Config.pid_rudd_cfg.scale = yaw_scale;
            PID_SetScaleFactor(&Airplane_Status.pid_rudd_servo,
                               Airplane_Config.pid_rudd_cfg.scale);
            break;

        default:
            break;
    }

    param_idx++;
    if(param_idx > 2)
        param_idx = 0;


#endif // AIRPLANE_PID_POT_TYPE

}

/**
 * Airplane_ChkFlyMode - Function to check current fly mode according to
 *                       the current input pulse width of RCIN_AUX1_IDX channel.
 *
 * #. This is a prototype function and should be reviewed later.
 *
 * @param   [input]     *p_rc_in        A "UNSIGNED" uint16_t array[RCIN_CH_TOTAL]
 *                                      contains values of RX signal pulse widths.
 *                                      (The unit should be timer tick).
 *
 * @return  [AIRPLANE_FLY_MODE]         Current fly mode.
 * @retval  [AIRPLANE_MANUAL_FLY]
 * @retval  [AIRPLANE_SELF_STABILIZE]
 * @retval  [AIRPLANE_RETURN_TO_HOME]
 *
 */
static AIRPLANE_FLY_MODE Airplane_ChkFlyMode(uint16_t *p_rc_in)
{
    AIRPLANE_FLY_MODE fly_mode;

    if(p_rc_in[RCIN_AUX1_IDX] >= TIMER1_MICROS_TO_TICKS(1800)){
        fly_mode = AIRPLANE_RETURN_TO_HOME;
    }
    else if(p_rc_in[RCIN_AUX1_IDX] >= TIMER1_MICROS_TO_TICKS(1300)){
        fly_mode = AIRPLANE_SELF_STABILIZE;
    }
    else if(p_rc_in[RCIN_AUX1_IDX] >= TIMER1_MICROS_TO_TICKS(900)){
        fly_mode = AIRPLANE_MANUAL_FLY;
    }
    else{
        fly_mode = AIRPLANE_RETURN_TO_HOME;
    }

    return fly_mode;
}

/**
 * Airplane_CalAngleDiff - Function to calculate the angle difference between current
 *                         angle and target angle.
 *
 * #. This is a prototype function and should be reviewed later.
 *
 * @param   [input]     current_angle
 * @param   [input]     target_angle
 * @param   [input]     max_angle
 * @param   [input]     min_angle
 *
 * @return  The angle difference.
 * @retval  [max_angle ~ min_angle]
 *
 */
static float Airplane_CalAngleDiff(float current_angle, float target_angle,
                                   float max_angle, float min_angle)
{
    float angle_diff;

    angle_diff = current_angle - target_angle;

    if(angle_diff > max_angle){
        angle_diff -= (max_angle * 2);
    }
    else if(angle_diff < min_angle){
        angle_diff -= (min_angle * 2);
    }

    return angle_diff;
}

/**
 * Airplane_TxMessage - Function to transmit FC status to external tool
 *                      via UART interface.
 *
 * @param   [in]        delta_time
 *
 * @return  [none]
 *
 */
static void Airplane_TxMessage(uint32_t delta_time)
{
    static uint32_t accum_delta_time = 0;
    static uint8_t mp_send_idx = 0;

#if AIRPLANE_STATUS_SNAPSHOT_EN
    static AIRPLANE_STATUS *p_current_status = &Airplane_StatusSnapshot;
#else
    static AIRPLANE_STATUS *p_current_status = &Airplane_Status;
#endif

    /*
     * Send out airplay status every 10ms sequentially.
     * 20 ms * 5 kinds status = 100 ms, 10Hz update rate.
     */
    accum_delta_time += delta_time;
    if(accum_delta_time >= 20000){

        switch(mp_send_idx){

            /* General status */
            case 0:

#if AIRPLANE_STATUS_SNAPSHOT_EN
                /* Store current completed status */
                memcpy((void *)&Airplane_StatusSnapshot, (void *)&Airplane_Status,
                       sizeof(Airplane_StatusSnapshot));
#endif

                MP_Send(MP_RSP_SYS_HEARTBEAT, (uint8_t *)&(p_current_status->heartbeat),
                        sizeof((p_current_status->heartbeat)));
                MP_Send(MP_RSP_SYS_GENERAL, (uint8_t *)&(p_current_status->general),
                        sizeof((p_current_status->general)));
                MP_Send(MP_RSP_SYS_SETPOINT, (uint8_t *)&(p_current_status->setpoint),
                        sizeof(p_current_status->setpoint));
                MP_Send(MP_RSP_SYS_CRUISE_STATE, (uint8_t *)&(p_current_status->current_cruise_state),
                        sizeof(p_current_status->current_cruise_state));
                MP_Send(MP_RSP_IN_CHANNELS, (uint8_t *)(p_current_status->rc_pulse_in),
                        sizeof(p_current_status->rc_pulse_in));
                MP_Send(MP_RSP_OUT_CHANNELS, (uint8_t *)(p_current_status->rc_pulse_out),
                        sizeof(p_current_status->rc_pulse_out));

                break;

            /* AHRS status */
            case 1:

                MP_Send(MP_RSP_AHRS_FULL, (uint8_t *)&(p_current_status->ahrs_data),
                        sizeof((p_current_status->ahrs_data)));
                break;

            /* PID status */
            case 2:
                MP_Send(MP_RSP_PID_VAL_ROLL, (uint8_t *)&(p_current_status->pid_aile_servo.value),
                        sizeof(p_current_status->pid_aile_servo.value));
                MP_Send(MP_RSP_PID_VAL_PITCH, (uint8_t *)&(p_current_status->pid_elev_servo.value),
                        sizeof(p_current_status->pid_elev_servo.value));
                MP_Send(MP_RSP_PID_VAL_YAW, (uint8_t *)&(p_current_status->pid_rudd_servo.value),
                        sizeof(p_current_status->pid_rudd_servo.value));
                MP_Send(MP_RSP_PID_VAL_BANK, (uint8_t *)&(p_current_status->pid_band_turn.value),
                        sizeof(p_current_status->pid_rudd_servo.value));
                break;

            /* PID configuration */
            case 3:

                MP_Send(MP_RSP_PID_CFG_ROLL, (uint8_t *)&(p_current_status->pid_aile_servo.config),
                        sizeof(p_current_status->pid_aile_servo.config));
                MP_Send(MP_RSP_PID_CFG_PITCH, (uint8_t *)&(p_current_status->pid_elev_servo.config),
                        sizeof(p_current_status->pid_elev_servo.config));
                MP_Send(MP_RSP_PID_CFG_YAW, (uint8_t *)&(p_current_status->pid_rudd_servo.config),
                        sizeof(p_current_status->pid_rudd_servo.config));
                MP_Send(MP_RSP_PID_CFG_BANK, (uint8_t *)&(p_current_status->pid_band_turn.config),
                        sizeof(p_current_status->pid_rudd_servo.config));
                break;

            /* GPS */
            case 4:

                MP_Send(MP_RSP_GPS_GENERAL, (uint8_t *)&Airplane_GPS.general,
                        sizeof(Airplane_GPS.general));
                MP_Send(MP_RSP_GPS_NMEA_GGA, (uint8_t *)&Airplane_GPS.nmea.gpgga,
                        sizeof(Airplane_GPS.nmea.gpgga));
                MP_Send(MP_RSP_GPS_NMEA_RMC, (uint8_t *)&Airplane_GPS.nmea.gprmc,
                        sizeof(Airplane_GPS.nmea.gprmc));
                MP_Send(MP_RSP_GPS_WAYPOINT, (uint8_t *)&Airplane_GPS.wpt,
                        sizeof(Airplane_GPS.wpt));
                MP_Send(MP_RSP_GPS_NAVIGATION, (uint8_t *)&Airplane_GPS.nav,
                        sizeof(Airplane_GPS.nav));
                MP_Send(MP_RSP_GPS_ERR_LOG, (uint8_t *)&GPS_ErrorLog,
                        sizeof(GPS_ErrorLog));

                break;

            default:
                break;
        }

        mp_send_idx++;
        if(mp_send_idx > 4)
            mp_send_idx = 0;

        accum_delta_time = 0;
    }
}

/**
 * Airplane_RxMessage - Function to receive FC message transmitted by external tool
 *                      via UART interface.
 *
 * @param   [none]
 * @return  [none]
 *
 */
static void Airplane_RxMessage()
{
    struct{
        uint16_t idx;
        IMU_SENSOR_DATA sensor;
        uint16_t delta_time;
    }mp_imu_sensor_frm;

    uint8_t rx_frm_buf[MP_RX_FRM_BUF_SIZE];
    uint8_t rx_frm_size;
    MP_FRAME_HDR *p_rx_hdr;
    AHRS_NED_ATTITUDE *p_ned_att;

    rx_frm_size = MP_Recv(rx_frm_buf, sizeof(rx_frm_buf));

    if(rx_frm_size){
        p_rx_hdr = (MP_FRAME_HDR *)rx_frm_buf;

        switch(p_rx_hdr->cmd){
            /*
             * Process simulating IMU (accelerometer, gyroscope)
             * data which is transmitted by FDM from PC.
             */
            case MP_REQ_IMU_SENSOR_DATA:

#if defined(IMU_SENSOR_FG_EN)

                if(p_rx_hdr->len == sizeof(mp_imu_sensor_frm)){

                    memcpy((void *)&mp_imu_sensor_frm, (void *)(rx_frm_buf + sizeof(MP_FRAME_HDR)),
                           sizeof(mp_imu_sensor_frm));

                    IMU_SENSOR_UPDATE_FROM_UART(mp_imu_sensor_frm.sensor.accel_raw,
                                                mp_imu_sensor_frm.sensor.gyro_raw);

                }
#endif

                break;

            /*
             * Process simulating attitude (NED angle) which is
             * generated and transmitted by FDM from PC.
             */
            case MP_REQ_NED_ANGLE_DATA:
#if defined(IMU_SENSOR_ANGLE_FROM_FG) && IMU_SENSOR_ANGLE_FROM_FG

                if(p_rx_hdr->len == sizeof(AHRS_NED_ATTITUDE)){

                    p_ned_att = (AHRS_NED_ATTITUDE *)(rx_frm_buf + sizeof(MP_FRAME_HDR));

                    AHRS_SetSimAngle(p_ned_att->roll_angle,
                                     p_ned_att->pitch_angle,
                                     p_ned_att->heading_angle);

                }
#endif
                break;

            default:
                break;
        }
    }
}
