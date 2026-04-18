#include <stdint.h>
#include QMK_KEYBOARD_H
#include "pointing_device.h"
#include "transactions.h"
#include "timer.h"
#include "keymap_japanese.h"

#define MHEN_FN  LT(1, JP_MHEN)
#define SPC_NUM  LT(2, KC_SPC)
#define SPC_SMBL LT(3, KC_SPC)
#define HENK_FN LT(4, JP_HENK)
#define Z_SFT LSFT_T(JP_Z)
#define SLSH_SFT RSFT_T(JP_SLSH)
#define A_CTL LCTL_T(JP_A)
#define MINS_CTL RCTL_T(JP_MINS)
#define X_SPR LGUI_T(JP_X)
#define DOT_SPR RGUI_T(JP_DOT)
#define C_META LALT_T(JP_C)
#define COMM_META RALT_T(JP_COMM)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        JP_Q,            JP_W,            JP_E,            JP_R,            JP_T,            JP_Y,            JP_U,            JP_I,            JP_O,            JP_P,
        A_CTL,           JP_S,            JP_D,            JP_F,            JP_G,            JP_H,            JP_J,            JP_K,            JP_L,            MINS_CTL,
        Z_SFT,           X_SPR,           C_META,          JP_V,            JP_B,            JP_N,            JP_M,            COMM_META,       DOT_SPR,         SLSH_SFT,
                                                           MHEN_FN,         SPC_NUM,         SPC_SMBL,        HENK_FN
    ),
    [1] = LAYOUT(
        KC_ESC,          KC_NO,           KC_END,          KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_BSPC,         KC_NO,
        KC_HOME,         MS_BTN2,         MS_BTN3,         MS_BTN1,         KC_NO,           KC_LEFT,         KC_DOWN,         KC_UP,           KC_RGHT,         KC_NO,
        KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_DEL,          KC_NO,           KC_NO,           KC_NO,           KC_NO,
                                                           KC_NO,           KC_NO,           KC_TAB,          KC_ENT
    ),
    [2] = LAYOUT(
        KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,
        JP_1,            JP_2,            JP_3,            JP_4,            JP_5,            JP_6,            JP_7,            JP_8,            JP_9,            JP_0,
        KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,
                                                           KC_NO,           KC_NO,           KC_TRNS,         KC_NO
    ),
    [3] = LAYOUT(
        JP_GRV,          JP_ASTR,         JP_AMPR,         KC_NO,           JP_TILD,         KC_NO,           JP_UNDS,         JP_LCBR,         JP_RCBR,         JP_PLUS,
        JP_AT,           JP_QUOT,         JP_DQUO,         JP_COLN,         KC_NO,           JP_HASH,         JP_SCLN,         JP_LPRN,         JP_RPRN,         KC_NO,
        KC_NO,           JP_EXLM,         JP_CIRC,         JP_PIPE,         JP_DLR,          KC_NO,           JP_PERC,         JP_LBRC,         JP_RBRC,         JP_BSLS,
                                                           KC_TRNS,         KC_TRNS,         KC_NO,           KC_NO
    ),
    [4] = LAYOUT(
        KC_NO,           KC_NO,           KC_F11,          KC_NO,           KC_F11,          KC_F12,          KC_NO,           KC_NO,           KC_NO,           KC_NO,
        KC_F1,           KC_F2,           KC_F3,           KC_F4,           KC_F5,           KC_F6,           KC_F7,           KC_F8,           KC_F9,           KC_F10,
        KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,
                                                           KC_NO,           KC_NO,           KC_NO,           KC_NO
    )
};

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {
        tap_code(clockwise ? KC_BRIU : KC_BRID);
    } else if (index == 1) {
        tap_code(clockwise ? KC_VOLU : KC_VOLD);
    }
    return false;
}

typedef struct {
    bool pressed;
} mute_state_t;

void mute_sync_slave_handler(uint8_t in_len, const void* in_data, uint8_t out_len, void* out_data) {
    mute_state_t *state = (mute_state_t*)out_data;
    state->pressed = !gpio_read_pin(GP13);
}

void keyboard_post_init_user(void) {
    gpio_set_pin_input_high(GP13);
    transaction_register_rpc(MUTE_SYNC, mute_sync_slave_handler);
}

void housekeeping_task_user(void) {
    if (!is_keyboard_master()) return;
    static uint32_t last = 0;
    static bool prev = false;
    if (timer_elapsed32(last) > 10) {
        last = timer_read32();
        mute_state_t state = {0};
        if (transaction_rpc_exec(MUTE_SYNC, 0, NULL, sizeof(state), &state)) {
            if (state.pressed && !prev) {
                tap_code(KC_MUTE);
            }
            prev = state.pressed;
        }
    }
}

report_mouse_t pointing_device_task_combined_user(report_mouse_t left_report, report_mouse_t right_report) {
    left_report.h = -left_report.x;
    left_report.v = left_report.y;
    left_report.x = 0;
    left_report.y = 0;
    right_report.x = right_report.x;
    right_report.y = right_report.y;
    return pointing_device_combine_reports(left_report, right_report);
}

static bool disable_retro = false;
static uint16_t retro_timer;
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if ((layer_state_is(2) || layer_state_is(3)) && record->event.pressed) {
        del_mods(MOD_MASK_SHIFT);
    }
    switch (keycode) {
        case A_CTL:
        case Z_SFT:
        case X_SPR:
        case C_META:
        case MINS_CTL:
        case SLSH_SFT:
        case DOT_SPR:
        case COMM_META:
            if (record->event.pressed) {
                retro_timer = timer_read();
                disable_retro = false;
            } else {
                if (timer_elapsed(retro_timer) > 500) {
                    disable_retro = true;
                }
            }
            break;
    }
    return true;
}

bool get_retro_tapping(uint16_t keycode, keyrecord_t *record) {
    if (disable_retro) {
        return false;
    }
    switch (keycode) {
        case Z_SFT:
        case SLSH_SFT:
        case A_CTL:
        case MINS_CTL:
        case X_SPR:
        case DOT_SPR:
        case C_META:
        case COMM_META:
            return true;
        default:
            return false;
    }
}

void suspend_wakeup_init_user(void) {
    clear_keyboard();
}
