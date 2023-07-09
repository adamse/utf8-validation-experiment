#include <stdint.h>

enum state {
    state_fail,     // 0b0000
    state_ok,    // 0b0001
    state_tail1,    // 0b0010
    state_tail2,    // 0b0011
    state_tail3,    // 0b0100
    state_utf8_3_1, // 0b0101
    state_utf8_3_2, // 0b0110
    state_utf8_4_1, // 0b0111
    state_utf8_4_2, // 0b1000 // 4 bits per state
    n_states,
};

void fill_table1(void);
uint8_t run1(uint8_t*, uint8_t*, enum state);

void fill_table2(void);
uint8_t run2(uint8_t*, uint8_t*, enum state);

void fill_table3(void);
uint8_t run3(uint8_t*, uint8_t*, enum state);

void fill_table4(void);
uint8_t run4(uint8_t*, uint8_t*, enum state);
uint8_t run5(uint8_t*, uint8_t*, enum state);
uint8_t run6(uint8_t*, uint8_t*, enum state);
