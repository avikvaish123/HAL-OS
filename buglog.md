# ECE 391 MP3 Bug Log - Group 77

## Checkpoint 1

### 1. Character printed on keypress release
 - Symptom: Extra character printed when keypress released
 - Difficulty to find: 30 minutes
 - Difficulty to solve: 1 line
 - Solution: check if the scancode is > 0

### 2. RTC Interupt is only displayed once when it should be repeated
 - Symptom: The screen is not refreshing when it should have been refreshing with different info
 - Difficulty to find: 1 hour
 - Difficulty to solve: 3 lines
 - Solution: Needed to read from status register C on the RTC to re-enable interupts

### 3. Exceptions weren't properly initialized in memory
 - Symptom: For all exception tests, printed only error message for first exception
 - Difficulty to find: 30 min
 - Difficulty to solve: 1 hour
 - Solution: Needed to initialize handlers for each indiviual exception, versus having a general exception handler

### 4. Wrong paramter check in i8259
 - Symptom: The enable and disable were just exectuing not running any of the code inside
 - Difficulty to find: 20 min
 - Difficulty to solve: 5 minutes
 - Solution: Changed conditions to filter out the invalid IRQ values

 
### 5. Interrupts could still happen after Exception
 - Symptom: In test case, could still type after raising exception
 - Difficulty to find: 10 min
 - Difficulty to fix: 20 min
 - Solution: looked at osdev to realize gate paramters for idt needed to be changed for exceptions from trap gate to intr gate

### 6. Incorrect sizes and address for paging
 - Symptom: When looking at info mem, the size and addr values were 2x larger than expected values
 - Difficulty to Find: 2 hours
 - Difficulty to Fix: 1 min
 - Solution: in struct, 32 bit value was actually an array of 2 elements versus just one 32 bit value

### 7. Page Table Wasn't initializing properly
 - Symptom: Compiler errors, unable to determine even after reviewing syntax why array is inaccessible and why certain fields were not initializing
 - Difficulty to Find: 25min
 - Difficulty to Fix: 5 min
 - Solution: Updated array initialization to match struct signature and also removed extern keywords from arrays which were causing issues with accessing it

### 8. Kernel is bootlooping after paging in enabled
 - Symptom: Interrupts, faults, or exceptions don't seem to be causing the bootloop.  Bootloop occurs at the x86 instruction directly after paging is enabled (new bit value pushed to cr0), gdb states error "Cannot access memory at address 0x400a9e".  Assuming memory access issues aren't allowing x86 function to ret, meaning kernel memory page isn't present
 - Difficulty to Find: 6 hours
 - Difficulty to Fix: 5 min
 - Solution:  Looking into info mem shows that the page sizes are correct, but that the starting address of each page is twice that of what it should be.  Assuming some value or pointer is being multiplied by 2 or has 1 more extra bit than it needs. *Update*: The issue was that val was initialized to 2 instead of 1, effectively doubling the size of the directory and table entries.  Changing 2 to 1 in the val field solved all issues.

## Checkpoint 2

### 1. Terminal printing on wrong line
 - Symptom: Terminal printing on line 1 instead of line 0
 - Difficulty to find: 1 minute
 - Difficulty to fix: 2 lines
 - Solution: reset cursor to line 0 when writing

### 2. Terminal write never executes
 - Symptom: After enter is pressed, nothing is on the screen
 - Difficulty to find: 5 minutes
 - Difficulty to fix: 2 lines
 - Solution: Reset current_char to 0 when keyboard_buffer is cleared

### 3. Characters were ghosting when printing on multiple lines
 - Symptom: Ghosting characters and weird ASCII (# 128)
 - Difficulty to find: 15 minutes
 - Difficulty to fix: 5 lines
 - Solution: Null-terminate strings to be printed to the screen

### 4. Cursor not moving properly
 - Symptom: Cursor disappears
 - Difficulty to find: 1 hour
 - Difficulty to fix: 2 lines
 - Solution: Set attribute byte of video memory to ATTRIB when clearing

### 5. Long file names not truncated
 - Symptom: Smiley face printed at the end of `verylargetextwithverylongfilename.tx`
 - Difficulty to find: 30 minutes
 - Difficulty to fix: 5 lines
 - Solution: Make all strings from `directory_read` null-terminated

### 6. RTC was only executing a single time
 - Symptom: Interrupt only executed once and was not getting called again
 - Difficulty to find: 30 minutes
 - Difficulty to fix: 2 lines
 - Solution: Had to allow interupts to start again

### 7. RTC was not ending even when calling RTC close
 - Symptom: Interrupts were still coming in and printing to screen even after RTC was closed
 - Difficulty to find: 30 minutes
 - Difficulty to fix: 1 lines
 - Solution: Called disable IRQ with the RTC IRQ line

## Checkpoint 3

### 1. Incorrect arguments passed to system calls
 - Symptom: Filename passed to open system call was incorrect
 - Difficulty to find: 2 minutes
 - Difficulty to fix: 2 lines
 - Solution: Do not push ESP in the assembly linkage

### 2. Execute page fault
 - Symptom: Execute page faults immediately
 - Difficulty to find: 10 minutes
 - Difficulty to fix: 1 line
 - Solution: Change allocated page size to 4 MiB from 4 KiB

### 3. Page Fault in Read
 - Symptom: when reading directory, page fault
 - Difficulty to find: 15 minutes
 - Difficulty to fix: 1 line
 - Solution: had wrong file type for directories

### 4. Terminal appearing on next line
 - Symptom: When using the shell, the typed letters appear on the next line
 - Difficulty to find: 5 minutes
 - Difficulty to fix: rewrite function
 - Solution: Keep track of if a printed string has a newline or not

### 5. Terminal wasn't properly print syserr
 - Symptom: when printing syserr overall tests pass, it didn't print properly
 - Difficulty to find: 30 min
 - Difficulty to fix: add conditional statement
 - Solution: keep track of number of enters in write buffer

### 6. Exit in Halt broke kernel
 - Symptom: Exception caused when exit called in base shell
 - Difficulty to find: 2 min
 - Difficulty to fix: move conditional and add line of code
 - Solution: instead of returning 0, execute a new shell

## Checkpoint 4

### 1. Shell stacking >2 does not work
 - Symptom: Whenever trying to exit twice in a row, the system reboots
 - Difficulty to find: 3 hours
 - Difficulty to fix: changing restore functions, fixing read_data
 - Solution: PCB was getting erased, so fixed functions to make sure kernel stack does not overflow

### 2. `cat frame1.txt` would not print the full text
 - Symptom: The text would be cut off near the top
 - Difficulty to find: 10 minutes
 - Difficulty to fix: 30 minutes
 - Solution: Added newline tracker to make space for text

### 3. Vidmap page fault
 - Symptom: Running `fish` would page fault
 - Difficulty to find: 1 minute
 - Difficulty to fix: 5 minutes
 - Solution: Remap pages correctly

## Checkpoint 5

### 1. Page Faulting when Implementing multiple buffers
 - Symptom: On start up, page faulting
 - Difficulty to find: 15 min
 - Difficulty to fix: 3 lines
 - Solution: adding initial if case if executing shell, set active terminal to 0

### 2. Cursor wasn't updating when switching terminal
 - Symptom: Cursor wasn't moving when clicking Alt+F
 - Difficulty to find: 20 min
 - Difficulty to fix: 5 min
 - Solution: Added another condition in terminal read to update cursor if terminal was changed

### 3. Video Memory Copying in Terminal Switching
 - Symptom: When switching terminals, previous text isn't being copied over
 - Difficulty to Find: 20 min
 - Difficulty to Fix: 2 min
 - Solution: had to swap dest and src in memcpy args

### 4. Keyboard not working after terminal switch
 - Symptom: Can't type on keyboard, even though IF flag is high
 - Difficulty to Find: 30 min
 - Difficulty to Fix: 5 min
 - Solution: When executing, we don't send EOI for keyboard, so pic was still masking it

### 5. Screen crashes when exiting the base shell in any terminal besides 1
 - Symptom: Screen is spammed with `391OS> ` characters
 - Difficulty to Find: 10 minutes
 - Difficulty to Fix: 1 line
 - Solution: Need to point `tss.esp0` to the correct kernel stack when relaunching shell

### 6. Page fault in shell stacking
 - Symptom: After exiting in T1, then switching to T2, then opening `shell` and exiting, there is a page fault
 - Difficulty to Find: 2 hours
 - Difficulty to Fix: 2 lines
 - Solution: We weren't updating the user page for the program whenever we switched terminals

### 7. Active Keyboard Buffer
 - Symptom: Keyboard buffer being written to all terminals, not active terminal
 - Difficulty to Find: 1 hour
 - Difficulty to Fix: Multiple lines, if statements, and critical sections
 - Solution: We weren't checking if (scheduled == active) in terminal read when we puts, added that condition in multiple places

### 8. Back space doesn't erase character from screen
 - Symptom: backspace was glitching and not removing characters
 - Difficulty to Find: 1 hour
 - Difficulty to Fix: Mulitple lines and functions
 - Solution: We were writing to different memory, not video memory, had to update screen_x and screen_y everytime we schedule switch 

### 9. Pingpong stalls
 - Symptom: After an amount time, pingpong stalls and RTC doesn't get an interrupt
 - Difficulty to Find: 30 min
 - Difficulty to Fix: 1 line
 - Solution: don't disable IRQ on rtc close because fish calls rtc close

### 10. On executing anything in T3, system crashes
 - Symptom: Executing in just T1 and T2 works fine, T3 doesn't when running programs that halt immediately
 - Difficulty to Find: 5 hours
 - Difficulty to Fix: 25 min
 - Solution: terminal read wasn't properly updating or synced across the multiple terminals

### 11. Interrupt Flags not set properly
 - Symptom: When executing a shell, exiting it, and then running counter, we no longer get kb interrupts until counter is finished
 - Difficulty to Find: 1 hour
 - Difficulty to Fix: 1 line
 - Solution: Didn't close out an cli() if removing base shell
