#include "fans_buzzer.h"

#include "mainboard_config.h"
#include "math.h"
#include "tim.h"

#include <string.h>

void fans_init() {
    pwm_set_period(&HTIM_PWM, PWM_FANS_STANDARD_PERIOD);
    fans_set_speed(0.15);
    pwm_start_channel(&HTIM_PWM, PWM_FANS_CHANNEL);
}
void fans_set_speed(float power) {
    if (power > 1 || power < 0)
        return;
    pwm_set_duty_cicle(&HTIM_PWM, PWM_FANS_CHANNEL, 1 - power);
}
void fans_set_speed_from_temp(float temp) {
    if (temp < 21)
        return fans_set_speed(0.15f);
    if (temp < 49)
        return fans_set_speed(0.75 / 30 * (temp - 20) + 0.15);
    fans_set_speed(1);
}

// credits to the master sborato PM Alex
////////////////////////////////////////////////////////////////////////////////

typedef enum {
    A0 = 0U,
    AS0,
    B0,
    C0,
    CS0,
    D0,
    DS0,
    E0,
    F0,
    FS0,
    G0,
    GS0,
    A1,
    AS1,
    B1,
    C1,
    CS1,
    D1,
    DS1,
    E1,
    F1,
    FS1,
    G1,
    GS1,
    A2,
    AS2,
    B2,
    C2,
    CS2,
    D2,
    DS2,
    E2,
    F2,
    FS2,
    G2,
    GS2,
    Pause,
    End
} NoteNameEnum;

uint16_t notes_freqs[] = {440,  466,  494,  523,  554,  587,  622,  659,  698,  740,  784,  831,
                          880,  932,  988,  1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661,
                          1760, 1865, 1976, 2093, 2217, 2349, 2439, 2637, 2794, 2960, 3136, 3322};

typedef struct {
    NoteNameEnum note;
    uint8_t beats;
} NoteTypeDef;

NoteTypeDef fra_martino[] = {
    {C0, 4},
    {D0, 4},
    {E0, 4},
    {C0, 4},
    {Pause, 1},
    {C0, 4},
    {D0, 4},
    {E0, 4},
    {C0, 4},
    {Pause, 1},
    {F0, 4},
    {G0, 4},
    {A1, 4},
    {Pause, 1},
    {F0, 4},
    {G0, 4},
    {A1, 4},
    {End, 0}};

NoteTypeDef gandalf[] = {{FS1, 4},   {Pause, 4}, {FS1, 2},   {FS1, 1},   {FS1, 1},   {E1, 1},    {FS1, 1},   {Pause, 2},
                         {FS1, 4},   {Pause, 4}, {FS1, 2},   {FS1, 1},   {FS1, 1},   {E1, 1},    {FS1, 1},   {FS1, 2},
                         {FS1, 2},   {Pause, 2}, {A2, 4},    {FS1, 2},   {Pause, 2}, {E1, 4},    {D1, 2},    {Pause, 2},
                         {B1, 2},    {B1, 2},    {CS1, 2},   {D1, 2},    {B1, 2},    {FS1, 2},   {FS1, 2},   {Pause, 4},
                         {FS1, 2},   {FS1, 1},   {FS1, 1},   {E1, 1},    {FS1, 1},   {Pause, 2}, {FS1, 2},

                         {FS1, 2},   {Pause, 4}, {FS1, 2},   {FS1, 1},   {FS1, 1},   {E1, 1},    {FS1, 1},   {FS1, 4},
                         {Pause, 2}, {A2, 4},    {FS1, 2},   {Pause, 2}, {E1, 4},    {D1, 3},    {Pause, 2}, {B1, 2},
                         {B1, 2},    {CS1, 2},   {D1, 2},    {B1, 2},    {FS1, 4},   {Pause, 4}, {FS1, 2},   {FS1, 1},
                         {FS1, 1},   {E1, 1},    {FS1, 1},   {Pause, 2}, {FS1, 4},   {Pause, 4}, {FS1, 2},   {FS1, 1},
                         {FS1, 1},   {E1, 1},    {FS1, 1},   {FS1, 2},   {Pause, 2}, {A2, 4},    {FS1, 2},   {Pause, 2},
                         {E1, 4},    {D1, 2},    {Pause, 2},

                         {B1, 2},    {B1, 2},    {CS1, 2},   {D1, 2},    {B1, 2},    {Pause, 2}, {Pause, 4},

                         {End, 0}};

NoteTypeDef sweep[] = {
    /*{A0, 10},
    {Pause, 4},
    {AS0, 10},
    {Pause, 4},
    {B0, 10},
    {Pause, 4},
    {C0, 10},
    {Pause, 4},
    {CS0, 10},
    {Pause, 4},
    {D0, 10},
    {Pause, 4},
    {DS0, 10},
    {Pause, 4},
    {E0, 10},
    {Pause, 4},
    {F0, 10},
    {Pause, 4},
    {FS0, 10},
    {Pause, 4},
    {G0, 10},
    {Pause, 4},
    {GS0, 10},
    {Pause, 4},
    {A1, 10},
    {Pause, 4},
    {AS1, 10},
    {Pause, 4},
    {B1, 10},
    {Pause, 4},
    {C1, 10},
    {Pause, 4},
    {CS1, 10},
    {Pause, 4},
    {D1, 10},
    {Pause, 4},
    {DS1, 10},
    {Pause, 4},
    {E1, 10},
    {Pause, 4},
    {F1, 10},
    {Pause, 4},
    {FS1, 10},
    {Pause, 4},
    {G1, 10},
    {Pause, 4},
    {GS1, 10},
    {Pause, 4},
    {A2, 10},
    {Pause, 4},*/
    {AS2, 10},
    {Pause, 4},
    //{B2, 10},
    //{Pause, 4},
    {C2, 10},
    {Pause, 4},
    //{CS2, 10},
    //{Pause, 4},
    {D2, 10},
    {Pause, 4},
    /*{DS2, 10},
    {Pause, 4},
    {E2, 10},
    {Pause, 4},
    {F2, 10},
    {Pause, 4},
    {FS2, 10},
    {Pause, 4},
    {G2, 10},
    {Pause, 4},
    {GS2, 10},
    {Pause, 4}*/
    {End, 1}};

uint16_t BPM = 600;

void _play_note(NoteTypeDef note, TIM_HandleTypeDef *htim) {
    uint16_t beat_duration_ms = 60 * 1000 / BPM;

    if (note.note != Pause) {
        pwm_generate_wave(htim, PWM_BUZZER_CHANNEL, PWM_SINE_WAVE, &notes_freqs[note.note], 1);
        HAL_Delay(beat_duration_ms * note.beats);
        pwm_stop_channel(htim, PWM_BUZZER_CHANNEL);
    } else {
        HAL_Delay(beat_duration_ms * note.beats);
    }

    HAL_Delay(20);
}

void BUZ_sborati(TIM_HandleTypeDef *htim) {
    NoteTypeDef *n = gandalf;

    while (n->note != End)
        _play_note(*n++, htim);

    pwm_stop_channel(htim, PWM_BUZZER_CHANNEL);
}