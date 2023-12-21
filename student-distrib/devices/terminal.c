#include "terminal.h"
#include "../lib.h"
#include "keyboard.h"
#include "../scheduler.h"
#include "../paging.h"
#include "../system_calls.h"

/* int32_t terminal_open()
 * Inputs:      void
 * Return Value: void
 * Function: Initializes terminal environment, including buffers and cursor position */
int32_t
terminal_open(const uint8_t* filename) {
    int i;
    int j;
    // set cursor to top of screen
    // init buffer and flags
    for (i = 0; i < MAX_TERMINALS; i++){
        terminals[i].current_char = 0;
        terminals[i].enter_flag = 0;
        terminals[i].current_line = 0;
        terminals[i].reset_flag = 0;
        terminals[i].char_in_line = 0;
        for (j = 0; j < MAX_BUFFER_LENGTH; j++) {
            terminals[i].keyboard_buffer[j] = NULL_ASCII;
        }
    }
    
    return -1;
}


/* int32_t terminal_close()
 * Inputs:      void
 * Return Value: void
 * Function: Closes the terminal environment */
int32_t
terminal_close(int32_t fd) {
    int i;
    int j;

    // reset buffer and flags
    for (i = 0; i < MAX_TERMINALS; i++){
        terminals[i].current_char = 0;
        terminals[i].enter_flag = 0;
        terminals[i].current_line = 0;
        terminals[i].reset_flag = 0;
        terminals[i].char_in_line = 0;
        for (j = 0; j < MAX_BUFFER_LENGTH; j++) {
            terminals[i].keyboard_buffer[j] = NULL_ASCII;
        }
    }

    return -1;
}


/* int32_t terminal_read(const void* buf, uint32_t nbytes)
 * Inputs:      buf - A pointer to the buffer where keyboard data will be stored
 *              nbytes - The maximum number of bytes to read
 * Return Value: The number of bytes read and stored in 'buf'
 * Function: Reads keyboard input and stores it in the provided buffer */
int32_t 
terminal_read(int32_t fd, void* buf, int32_t nbytes) {
    int i;
    int last_terminal = active_terminal;
    int prev_char_in_line = terminals[active_terminal].char_in_line;
    int shift_up_flag = 0;

    // wait until enter or max characters
    while (!terminals[active_terminal].enter_flag) {
        // if we have typed a character or reset the screen
        if (terminals[active_terminal].current_char != terminals[active_terminal].last_current_char || terminals[active_terminal].reset_flag || active_terminal != last_terminal) {
            terminals[active_terminal].reset_flag = 0;
            // shift the screen if required
            if (terminals[active_terminal].current_char >= (NUM_COLS - terminals[active_terminal].char_in_line)) {
                if (!shift_up_flag && terminals[active_terminal].current_line >= NUM_ROWS - 1) {
                    if (scheduled_terminal == active_terminal) {
                        cli();
                        shift_video_mem_up(0, NUM_ROWS);
                        sti();
                    }
                    
                    terminals[active_terminal].current_line--;
                    shift_up_flag = 1;
                }
            }
            terminals[active_terminal].last_current_char = terminals[active_terminal].current_char;
            last_terminal = active_terminal;
            terminals[active_terminal].keyboard_buffer[terminals[active_terminal].current_char] = NULL_ASCII;
            
            // print out the keyboard buffer if we are currently running on our active terminal
            while(1) {
                if (scheduled_terminal == active_terminal) {
                    cli();
                    set_cursor(terminals[active_terminal].char_in_line, terminals[active_terminal].current_line);
                    puts(terminals[active_terminal].keyboard_buffer);
                    sti();
                    break;
                }
            }
        }
    }

    terminals[active_terminal].enter_flag = 0;

    if (terminals[active_terminal].current_char > (NUM_COLS - prev_char_in_line)) {
        terminals[active_terminal].current_line++;
        set_cursor(0, terminals[active_terminal].current_line);
    }

    // add newline character at the end of the keyboard buffer
    terminals[active_terminal].keyboard_buffer[terminals[active_terminal].current_char] = NEWLINE_ASCII;
    terminals[active_terminal].current_char++;

    // pass copy keyboard buffer into buf
    for (i = 0; i < terminals[active_terminal].current_char; i++) {
        ((char*)buf)[i] = terminals[active_terminal].keyboard_buffer[i];
    }
    return terminals[active_terminal].current_char;
}


/* int32_t terminal_write(const void* buf, uint32_t nbytes)
 * Inputs:      buf - A pointer to the data to be written to the terminal
 *              nbytes - The number of bytes to write
 * Return Value: The number of bytes written
 * Function: Writes data from the buffer to the terminal screen */
int32_t
terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
    int i;
    int str_len = 0;
    int newline_count = 0;
    int lines_left = NUM_ROWS - 1;

    // parameter checking
    if (nbytes > MAX_INPUT_BUF_LEN) {
        return 0;
    }

    // initial buf check
    for (i = 0; i < nbytes; i++) {
        str_len++;
        if (((char*)buf)[i] == NULL_ASCII) {
            break;
        } else if (((char*)buf)[i] == NEWLINE_ASCII) {
            newline_count += 1;
        }
    }

    // fill keyboard buffer with null char
    for (i = 0; i < MAX_BUFFER_LENGTH; i++) {
        terminals[scheduled_terminal].keyboard_buffer[i] = NULL_ASCII;
    }

    if (terminals[scheduled_terminal].current_char > 0) {
        // increment the current line to print the terminal buffer
        terminals[scheduled_terminal].current_line++;
        terminals[scheduled_terminal].current_char = 0;
    }    

    // if at the bottom of the screen, move the video memory up
    while (terminals[scheduled_terminal].current_line > NUM_ROWS - 1) {
        shift_video_mem_up(0, NUM_ROWS);
        terminals[scheduled_terminal].current_line--;
    }

    // if at the bottom of the screen and we need 2 lines to print, shift up
    if (terminals[scheduled_terminal].current_line == NUM_ROWS - 1 && nbytes >= (NUM_COLS - terminals[scheduled_terminal].char_in_line)) {
        shift_video_mem_up(0, NUM_ROWS);
        terminals[scheduled_terminal].current_line--;
    }

    lines_left = NUM_ROWS - terminals[scheduled_terminal].current_line;

    // make room for the text
    while (newline_count > lines_left) {
        shift_video_mem_up(0, NUM_ROWS);
        terminals[scheduled_terminal].current_line--;

        lines_left = NUM_ROWS - terminals[scheduled_terminal].current_line;
    }
   
    // print the terminal buffer
    set_cursor(terminals[scheduled_terminal].char_in_line, terminals[scheduled_terminal].current_line);
    puts((int8_t*)buf);

    if (nbytes != NUM_COLS && ((int8_t*)buf)[nbytes-1] != '\n') {
        terminals[scheduled_terminal].char_in_line += nbytes;
    } else {
        nbytes--;
    }

    // shift video memory to make room for the new typing
    if (newline_count) {
        int j;
        for (j = 0; j < newline_count; j++){
            terminals[scheduled_terminal].current_line++;
        }
        if (nbytes >= (NUM_COLS-terminals[scheduled_terminal].char_in_line)) {
            terminals[scheduled_terminal].current_line++;
        }

        while (terminals[scheduled_terminal].current_line > NUM_ROWS - 1) {
            shift_video_mem_up(0, NUM_ROWS);
            terminals[scheduled_terminal].current_line--;
        }
        terminals[scheduled_terminal].char_in_line = 0;
    }

    set_cursor(terminals[scheduled_terminal].char_in_line, terminals[scheduled_terminal].current_line);

    return str_len;
}

/* void update_kb_buffer(char input)
 * Inputs:      input - The character to update the keyboard buffer with
 * Return Value: void
 * Function: Updates the keyboard buffer with the input character */
void
update_kb_buffer(char input) {
    // if newline is pressed/has been pressed, don't write
    if ((input == NEWLINE_ASCII) || (terminals[active_terminal].enter_flag)) {
        terminals[active_terminal].enter_flag = 1;
        terminals[active_terminal].char_in_line = 0;
        return;
    } else if (input == BACKSPACE_ASCII) {
        // handle backspace character
        if (terminals[active_terminal].current_char > 0) {
            terminals[active_terminal].current_char--;
        }
        terminals[active_terminal].keyboard_buffer[terminals[active_terminal].current_char] = NULL_ASCII;
    
        // set page to write to video memory
        tableArray[VIDEO_MEMORY_PAGE].offset_31_12 = VIDEO >> PAGING_OFFSET;
        vmemTableArray[VIDMAP_VMEM_LOC_PAGE].offset_31_12 = VIDEO >> PAGING_OFFSET;
        flush_tlb();

        set_cursor(terminals[active_terminal].terminal_screen_x, terminals[active_terminal].terminal_screen_y);

        backspace_called();

        // reset video page to point to scheduled terminal
        tableArray[VIDEO_MEMORY_PAGE].offset_31_12 = (TERMINAL_VIDEO + (ALIGN_4KB*scheduled_terminal)) >> PAGING_OFFSET;
        vmemTableArray[VIDMAP_VMEM_LOC_PAGE].offset_31_12 = (TERMINAL_VIDEO + (ALIGN_4KB*scheduled_terminal)) >> PAGING_OFFSET;
        flush_tlb();

        set_cursor(terminals[scheduled_terminal].terminal_screen_x, terminals[scheduled_terminal].terminal_screen_y);  
        return;
    } else if (terminals[active_terminal].current_char >= MAX_BUFFER_LENGTH - 1) {
        // if buffer is full, don't write
        return;
    } else {
        // put input into keyboard buffer
        terminals[active_terminal].keyboard_buffer[terminals[active_terminal].current_char] = input;
        terminals[active_terminal].current_char++;
    }
}

/* void clear_terminal()
 * Inputs:      None
 * Return Value: void
 * Function: Clears the screen after CTRL-L */
void
clear_terminal() {
     cli();
    // set page to write to video memory
    tableArray[VIDEO_MEMORY_PAGE].offset_31_12 = VIDEO >> PAGING_OFFSET;
    vmemTableArray[VIDMAP_VMEM_LOC_PAGE].offset_31_12 = VIDEO >> PAGING_OFFSET;
    flush_tlb();

    clear();
    set_cursor(0, 0);

    // reset video page to point to scheduled terminal
    tableArray[VIDEO_MEMORY_PAGE].offset_31_12 = (TERMINAL_VIDEO + (ALIGN_4KB*scheduled_terminal)) >> PAGING_OFFSET;
    vmemTableArray[VIDMAP_VMEM_LOC_PAGE].offset_31_12 = (TERMINAL_VIDEO + (ALIGN_4KB*scheduled_terminal)) >> PAGING_OFFSET;
    flush_tlb();
    sti();

    terminals[active_terminal].reset_flag = 1;
    terminals[active_terminal].current_line = 0;
    terminals[active_terminal].char_in_line = 0;
}
