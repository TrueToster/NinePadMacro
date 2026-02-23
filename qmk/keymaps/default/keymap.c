// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "raw_hid.h"
#include <string.h>

enum layers {
    layer1 = 0,
    layer2,
    layer3
};

enum custom_keycodes {
    F3G = SAFE_RANGE,
    F3B,
    F3H,
    F3I,
    F3T,
    F3A,
    F3P,
    F3C,        
    F3SH,
    F3ALTSH
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
     [layer1] = LAYOUT_ortho_3x3(
        KC_F13, KC_F14, KC_F15,
        KC_F16, KC_F17, KC_F18,
        QK_BOOT, TO(layer3), TO(layer2)
    ),
    [layer2] = LAYOUT_ortho_3x3(
        F3G, F3B, F3H,
        F3T, F3A, F3SH,
        F3ALTSH, TO(layer1), TO(layer3)
    ),
    [layer3] = LAYOUT_ortho_3x3(
        KC_F19, KC_F20, KC_F21,
        KC_F22, KC_F23, KC_F24,
        KC_PSCR, TO(layer2), TO(layer1)
    )
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    case F3G:  // F3 + G
        if (record->event.pressed) {
            register_code(KC_F3);
            register_code(KC_G);
        } else {
            unregister_code(KC_F3);
            unregister_code(KC_G);
        }
        return false;
        
    case F3B:  // F3 + B
        if (record->event.pressed) {
            register_code(KC_F3);
            register_code(KC_B);
        } else {
            unregister_code(KC_F3);
            unregister_code(KC_B);
        }
        return false;
        
    case F3H:  // F3 + H
        if (record->event.pressed) {
            register_code(KC_F3);
            register_code(KC_H);
        } else {
            unregister_code(KC_F3);
            unregister_code(KC_H);
        }
        return false;
        
    case F3I:  // F3 + I
        if (record->event.pressed) {
            register_code(KC_F3);
            register_code(KC_I);
        } else {
            unregister_code(KC_F3);
            unregister_code(KC_I);
        }
        return false;
        
    case F3T:  // F3 + T
        if (record->event.pressed) {
            register_code(KC_F3);
            register_code(KC_T);
        } else {
            unregister_code(KC_F3);
            unregister_code(KC_T);
        }
        return false;
        
    case F3A:  // F3 + A
        if (record->event.pressed) {
            register_code(KC_F3);
            register_code(KC_A);
        } else {
            unregister_code(KC_F3);
            unregister_code(KC_A);
        }
        return false;
        
    case F3P:  // F3 + P
        if (record->event.pressed) {
            register_code(KC_F3);
            register_code(KC_P);
        } else {
            unregister_code(KC_F3);
            unregister_code(KC_P);
        }
        return false;
        
    case F3C: //F3 + C
        if (record->event.pressed) {
            register_code(KC_F3);
            register_code(KC_C);
        } else { 
            unregister_code(KC_F3);
            unregister_code(KC_C);
        }
        return false;

    case F3SH:  // F3 + Shift
        if (record->event.pressed) {
            register_code(KC_F3);
            register_code(KC_LSFT);
        } else {
            unregister_code(KC_F3);
            unregister_code(KC_LSFT);
        }
        return false;
        
    case F3ALTSH:  // F3 + Alt + Shift
        if (record->event.pressed) {
            register_code(KC_F3);
            register_code(KC_LALT);
            register_code(KC_LSFT);
        } else {
            unregister_code(KC_F3);
            unregister_code(KC_LALT);
            unregister_code(KC_LSFT);
        }
        return false;
    }
    return true;
}

void raw_hid_receive(uint8_t *data, uint8_t length) { }

layer_state_t layer_state_set_user(layer_state_t state) {
    uint8_t data[2] = {0x01, get_highest_layer(state)};
    raw_hid_send(data, 2);
    return state;
}