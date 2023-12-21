#include "keyboard.h"
#include "../lib.h"
#include "../i8259.h"
#include "terminal.h"
#include "../scheduler.h"

// LUT for translating scan code set 1 to lowercase ASCII characters
const char lowercase_scancode_to_char [NUM_SCAN_CODES] = {
    '\0', // padding
    '\0', // escape
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    '\b', // backspace
    '\0', // tab
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n', // enter
    '\0', // left ctrl
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    '\0', // left shift
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    '\0', // right shift
    '\0', // keypad *
    '\0', // left alt
    ' ', // space
};

// LUT for translating scan code set 1 to uppercase ASCII characters
const char uppercase_scancode_to_char [NUM_SCAN_CODES] = {
    '\0', // padding
    '\0', // escape
    '!',
    '@',
    '#',
    '$',
    '%',
    '^',
    '&',
    '*',
    '(',
    ')',
    '_',
    '+',
    '\b', // backspace
    '\0', // tab
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    '{',
    '}',
    '\n', // enter
    '\0', // left ctrl
    'A',
    'S',
    'D',
    'F',
    'G',
    'H',
    'J',
    'K',
    'L',
    ':',
    '"',
    '~',
    '\0', // left shift
    '|',
    'Z',
    'X',
    'C',
    'V',
    'B',
    'N',
    'M',
    '<',
    '>',
    '?',
    '\0', // right shift
    '\0', // keypad *
    '\0', // left alt
    ' ', // space
};

volatile uint8_t caps_lock_flag;
volatile uint8_t l_shift_flag;
volatile uint8_t r_shift_flag;
volatile uint8_t ctrl_flag;
volatile uint8_t alt_flag;

/* void keyboard_init()
 * Inputs:      void
 * Return Value: void
 * Function: enables interrupts for the keyboard */
void
keyboard_init() {
    caps_lock_flag = 0x00;
    l_shift_flag = 0x00;
    r_shift_flag = 0x00;
    ctrl_flag = 0x00;
    alt_flag = 0x00;
    enable_irq(KEYBOARD_IRQ);
}

/* void keyboard_close()
 * Inputs: None
 * Outputs: None
 * Function: Masks interrupts from the keyboard */
void
keyboard_close() {
    disable_irq(KEYBOARD_IRQ);
}


/* void keyboard_handler()
 * Inputs:      void
 * Return Value: void
 * Function: prints echo of key pressed */
void
keyboard_handler() {
    uint8_t keyboard_status;
    uint8_t scan_code;
    uint8_t output_char;
    uint8_t use_caps;
    int i;

    cli();
    // check if output buffer is ready
    keyboard_status = inb(KEYBOARD_STATUS_PORT);
    if (keyboard_status & KEYBOARD_OUTPUT_BUFFER_STATUS_MASK) {
        // read scancode from keyboard port
        scan_code = inb(KEYBOARD_DATA_PORT);

        // handle special keys
        if (scan_code == CAPS_LOCK) {
            caps_lock_flag = caps_lock_flag ^ CAPS_LOCK_BITMASK;
        } else if (scan_code == L_SHIFT_PRESS) {
            l_shift_flag = 1;
        } else if (scan_code == R_SHIFT_PRESS) {
            r_shift_flag = 1;
        } else if (scan_code == L_SHIFT_RELEASE) {
            l_shift_flag = 0;
        } else if (scan_code == R_SHIFT_RELEASE) {
            r_shift_flag = 0;
        } else if (scan_code == CTRL_PRESS) {
            ctrl_flag = 1;
        } else if (scan_code == CTRL_RELEASE) {
            ctrl_flag = 0;
        } else if (scan_code == ALT_PRESS) {
            alt_flag = 1;
        } else if (scan_code == ALT_RELEASE) {
            alt_flag = 0;
        } else if (scan_code == L_PRESS && ctrl_flag) {
            // clear old commands from terminal
            clear_terminal();
        } else if (alt_flag && (scan_code == F1_PRESS || scan_code == F2_PRESS || scan_code == F3_PRESS)) {
            // switch terminal
            switch (scan_code) {
                case F1_PRESS:
                    terminal_switch(0);
                break;
                case F2_PRESS:
                    terminal_switch(1);
                break;
                case F3_PRESS:
                    terminal_switch(2);
                break;
                default:
                break;
            }
        } else if (scan_code == TAB_PRESS) {
            for (i = 0; i < TAB_SPACE; i++) {
                update_kb_buffer(SPACE_ASCII);
            }
        } else {
            if (scan_code <= NUM_SCAN_CODES) {
                // set flag for special characters
                use_caps = l_shift_flag | r_shift_flag;

                // if key is a letter, check caps lock
                if (((scan_code >= UPPER_ROW_LETTER_START) & (scan_code <= UPPER_ROW_LETTER_END)) | 
                    ((scan_code >= MID_ROW_LETTER_START) & (scan_code <= MID_ROW_LETTER_END)) | 
                    ((scan_code >= LOWER_ROW_LETTER_START) & (scan_code <= LOWER_ROW_LETTER_END))) {
                    use_caps ^= caps_lock_flag;
                }

                // handle uppercase/special characters
                if (use_caps) {
                    output_char = uppercase_scancode_to_char[scan_code];
                } else {
                    output_char = lowercase_scancode_to_char[scan_code];
                }

                // write to the keyboard buffer
                update_kb_buffer(output_char);
            }
        }
        
    }
    sti();
    send_eoi(KEYBOARD_IRQ);
}
