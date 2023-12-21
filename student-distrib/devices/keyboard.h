#define KEYBOARD_INDEX      0x21

#define KEYBOARD_STATUS_PORT                    0x64
#define KEYBOARD_OUTPUT_BUFFER_STATUS_MASK      0x01
#define KEYBOARD_DATA_PORT                      0x60
#define KEYBOARD_IRQ                            1
#define NUM_SCAN_CODES                          58
#define RELEASE_SCAN_CODES                      0x80

#define UPPER_ROW_LETTER_START                  0x10
#define UPPER_ROW_LETTER_END                    0x19
#define MID_ROW_LETTER_START                    0x1E
#define MID_ROW_LETTER_END                      0x26
#define LOWER_ROW_LETTER_START                  0x2C
#define LOWER_ROW_LETTER_END                    0x32

#define CAPS_LOCK                               0x3A
#define L_SHIFT_PRESS                           0x2A
#define L_SHIFT_RELEASE                         0xAA
#define R_SHIFT_PRESS                           0x36
#define R_SHIFT_RELEASE                         0xB6

#define CTRL_PRESS                              0x1D
#define CTRL_RELEASE                            0x9D

#define ALT_PRESS                               0x38
#define ALT_RELEASE                             0xB8
#define F1_PRESS                                0x3B
#define F2_PRESS                                0x3C
#define F3_PRESS                                0x3D

#define CAPS_LOCK_BITMASK                       0x01

#define L_PRESS                                 0x26
#define TAB_PRESS                               0x0F
#define TAB_SPACE                               4
#define SPACE_ASCII                             0x20

#define NUM_ROWS                25
#define TERMINAL_ROW_START      2


/* enable keyboard input */
void keyboard_init();

/* disable keyboard input */
void keyboard_close();

/* handle keyboard input to terminal */
void keyboard_handler();
