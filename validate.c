// experiment with "shift based DFA"[1] for utf8 validation
//
// [1]: https://gist.github.com/pervognsen/218ea17743e1442e59bb60d29b1aa725
//
// utf-8: https://datatracker.ietf.org/doc/html/rfc3629#section-4
//
//    UTF8-octets = *( UTF8-char )
//    UTF8-char   = UTF8-1 / UTF8-2 / UTF8-3 / UTF8-4
//
//    UTF8-1      = %x00-7F
//
//    UTF8-2      = %xC2-DF UTF8-tail
//
//    UTF8-3      = %xE0 %xA0-BF UTF8-tail
//                  / %xE1-EC 2( UTF8-tail )
//                  / %xED %x80-9F UTF8-tail
//                  / %xEE-EF 2( UTF8-tail )
//
//    UTF8-4      = %xF0 %x90-BF 2( UTF8-tail )
//                  / %xF1-F3 3( UTF8-tail )
//                  / %xF4 %x80-8F 2( UTF8-tail )
//
//    UTF8-tail   = %x80-BF
//
//
// start:
// 0xxxxxxx // utf8-1
// 0x00..0x80 -> start
//
// 110xxxxx // utf8-2
// 0xc2..0xe0 -> tail1
//
// 1110xxxx // utf8-3
// 0xe0 -> utf8-3-1
// 0xe1..0xed -> tail2
// 0xed -> utf8-3-2
// 0xee..0xf0 -> tail2
//
// 11110xxx // utf8-4
// 0xf0 -> utf8-4-1
// 0xf1..0xf4 -> tail3
// 0xf4 -> utf8-4-2
//
// utf8-3-1:
// 0xa0..0xc0 -> tail1
//
// utf8-3-2:
// 0x80..0xa0 -> tail1
//
// utf8-4-1:
// 0x90..0xc0 -> tail2
//
// utf8-4-2:
// 0x80..0x90 -> tail2
//
// tail1:
// 0x80..0xc0 -> start
//
// tail2:
// 0x80..0xc0 -> tail1
//
// tail3:
// 0x80..0xc0 -> tail2
//
// fail:
// _ -> fail

#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#include "validate.h"

// ordinary table-based DFA
// ------------------------

static uint8_t table1[n_states][256] = {0};

void fill_table1(void) {
#define CASE(match, current, next) case match: table1[current][i] = next; break;
#define FAIL(current) default: table1[current][i] = state_fail; break;
    // start
    for (int i = 0; i < 256; i++) {
        switch (i) {
            // utf8-1
            CASE(0x00 ... 0x7f, state_ok, state_ok);
            // utf8-2
            CASE(0xc2 ... 0xdf, state_ok, state_tail1);
            // utf8-3
            CASE(0xe0, state_ok, state_utf8_3_1);
            CASE(0xe1 ... 0xec, state_ok, state_tail2);
            CASE(0xed, state_ok, state_utf8_3_2);
            CASE(0xee ... 0xef, state_ok, state_tail2);
            // utf8-4
            CASE(0xf0, state_ok, state_utf8_4_1);
            CASE(0xf1 ... 0xf3, state_ok, state_tail3);
            CASE(0xf4, state_ok, state_utf8_4_2);
            // fail
            FAIL(state_ok);
        }
    }
    // utf8-3-1
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0xa0 ... 0xbf, state_utf8_3_1, state_tail1);
            FAIL(state_utf8_3_1);
        }
    }
    // utf8-3-2
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0x9f, state_utf8_3_2, state_tail1);
            FAIL(state_utf8_3_2);
        }
    }
    // utf8-4-1
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x90 ... 0xbf, state_utf8_4_1, state_tail2);
            FAIL(state_utf8_4_1);
        }
    }
    // utf8-4-2
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0x8f, state_utf8_4_2, state_tail2);
            FAIL(state_utf8_4_2);
        }
    }
    // tail1
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0xbf, state_tail1, state_ok);
            FAIL(state_tail1);
        }
    }
    // tail2
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0xbf, state_tail2, state_tail1);
            FAIL(state_tail2);
        }
    }
    // tail3
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0xbf, state_tail3, state_tail2);
            FAIL(state_tail3);
        }
    }
    // fail
    for (int i = 0; i < 256; i++) {
        switch (i) {
            FAIL(state_fail);
        }
    }
#undef CASE
#undef FAIL
}

uint8_t run1(uint8_t* start, uint8_t* end, enum state state) {
    for (uint8_t* s = start; s != end; s++) {
        state = table1[state][*s];
    }
    return state;
}


// flipped table
// -------------

static uint8_t table2[256][n_states] = {0};

void fill_table2(void) {
#define CASE(match, current, next) case match: table2[i][current] = next; break;
#define FAIL(current) default: table2[i][current] = state_fail; break;
    // start
    for (int i = 0; i < 256; i++) {
        switch (i) {
            // utf8-1
            CASE(0x00 ... 0x7f, state_ok, state_ok);
            // utf8-2
            CASE(0xc2 ... 0xdf, state_ok, state_tail1);
            // utf8-3
            CASE(0xe0, state_ok, state_utf8_3_1);
            CASE(0xe1 ... 0xec, state_ok, state_tail2);
            CASE(0xed, state_ok, state_utf8_3_2);
            CASE(0xee ... 0xef, state_ok, state_tail2);
            // utf8-4
            CASE(0xf0, state_ok, state_utf8_4_1);
            CASE(0xf1 ... 0xf3, state_ok, state_tail3);
            CASE(0xf4, state_ok, state_utf8_4_2);
            // fail
            FAIL(state_ok);
        }
    }
    // utf8-3-1
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0xa0 ... 0xbf, state_utf8_3_1, state_tail1);
            FAIL(state_utf8_3_1);
        }
    }
    // utf8-3-2
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0x9f, state_utf8_3_2, state_tail1);
            FAIL(state_utf8_3_2);
        }
    }
    // utf8-4-1
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x90 ... 0xbf, state_utf8_4_1, state_tail2);
            FAIL(state_utf8_4_1);
        }
    }
    // utf8-4-2
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0x8f, state_utf8_4_2, state_tail2);
            FAIL(state_utf8_4_2);
        }
    }
    // tail1
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0xbf, state_tail1, state_ok);
            FAIL(state_tail1);
        }
    }
    // tail2
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0xbf, state_tail2, state_tail1);
            FAIL(state_tail2);
        }
    }
    // tail3
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0xbf, state_tail3, state_tail2);
            FAIL(state_tail3);
        }
    }
    // fail
    for (int i = 0; i < 256; i++) {
        switch (i) {
            FAIL(state_fail);
        }
    }
#undef CASE
#undef FAIL
}

uint8_t run2(uint8_t* start, uint8_t* end, enum state state) {
    for (uint8_t* s = start; s != end; s++) {
        state = table2[*s][state];
    }
    return state;
}


// first shift based dfa
// ---------------------

enum { BITS_PER_STATE = 4, };

uint64_t table3[256];
// we have 9 states, so we need 9 * BITS_PER_STATE = 36 bits to encode the transitions
//
// ex transition 0 => 1 looks like this: 0x1, and is placed in the first nibble (little endian)
//
// table[i] |= (next & ((1 << BITS_PER_STATE) - 1) << (current * BITS_PER_STATE)
//
// to extract:
//
// (row >> current * BITS_PER_STATE) & ((1 << BITS_PER_STATE) - 1)
//


void fill_table3(void) {
#define INSERT(current, next) (((uint64_t)next & ((1 << BITS_PER_STATE) - 1)) << (current * BITS_PER_STATE))
#define CASE(match, current, next) case match: table3[i] |= INSERT(current, next); break;
#define FAIL(current) default: table3[i] |= INSERT(current, state_fail); break;
    // start
    for (int i = 0; i < 256; i++) {
        switch (i) {
            // utf8-1
            CASE(0x00 ... 0x7f, state_ok, state_ok);
            // utf8-2
            CASE(0xc2 ... 0xdf, state_ok, state_tail1);
            // utf8-3
            CASE(0xe0, state_ok, state_utf8_3_1);
            CASE(0xe1 ... 0xec, state_ok, state_tail2);
            CASE(0xed, state_ok, state_utf8_3_2);
            CASE(0xee ... 0xef, state_ok, state_tail2);
            // utf8-4
            CASE(0xf0, state_ok, state_utf8_4_1);
            CASE(0xf1 ... 0xf3, state_ok, state_tail3);
            CASE(0xf4, state_ok, state_utf8_4_2);
            // fail
            FAIL(state_ok);
        }
    }
    // utf8-3-1
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0xa0 ... 0xbf, state_utf8_3_1, state_tail1);
            FAIL(state_utf8_3_1);
        }
    }
    // utf8-3-2
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0x9f, state_utf8_3_2, state_tail1);
            FAIL(state_utf8_3_2);
        }
    }
    // utf8-4-1
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x90 ... 0xbf, state_utf8_4_1, state_tail2);
            FAIL(state_utf8_4_1);
        }
    }
    // utf8-4-2
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0x8f, state_utf8_4_2, state_tail2);
            FAIL(state_utf8_4_2);
        }
    }
    // tail1
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0xbf, state_tail1, state_ok);
            FAIL(state_tail1);
        }
    }
    // tail2
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0xbf, state_tail2, state_tail1);
            FAIL(state_tail2);
        }
    }
    // tail3
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0xbf, state_tail3, state_tail2);
            FAIL(state_tail3);
        }
    }
    // fail
    for (int i = 0; i < 256; i++) {
        switch (i) {
            FAIL(state_fail);
        }
    }
#undef CASE
#undef FAIL
#undef INSERT
}

uint8_t run3(uint8_t* start, uint8_t* end, enum state state) {
    for (uint8_t* s = start; s != end; s++) {
        uint64_t row = table3[*s];
        state = (row >> state * BITS_PER_STATE) & ((1 << BITS_PER_STATE) - 1);
    }
    return state;
}


// second shift based dfa
// ----------------------
//
// this one pre-multiplies BITS_PER_STATE2 in the table

enum { BITS_PER_STATE2 = 6, };

uint64_t table4[256];
// we have 9 states, so we need 9 * BITS_PER_STATE2 = 54 bits to encode the transitions
//
// ex transition 0 => 1 looks like this: 0x1, and is placed in the first nibble (little endian)
//
// table[i] |= (next * BITS_PER_STATE2 & ((1 << BITS_PER_STATE2) - 1) << (current * BITS_PER_STATE2)
//
// to extract:
//
// (row >> current * BITS_PER_STATE2) & ((1 << BITS_PER_STATE2) - 1)
//


void fill_table4(void) {
#define INSERT(current, next) (((uint64_t)(next * BITS_PER_STATE2) & ((1 << BITS_PER_STATE2) - 1)) << (current * BITS_PER_STATE2))
#define CASE(match, current, next) case match: table4[i] |= INSERT(current, next); break;
#define FAIL(current) default: table4[i] |= INSERT(current, state_fail); break;
    // start
    for (int i = 0; i < 256; i++) {
        switch (i) {
            // utf8-1
            CASE(0x00 ... 0x7f, state_ok, state_ok);
            // utf8-2
            CASE(0xc2 ... 0xdf, state_ok, state_tail1);
            // utf8-3
            CASE(0xe0, state_ok, state_utf8_3_1);
            CASE(0xe1 ... 0xec, state_ok, state_tail2);
            CASE(0xed, state_ok, state_utf8_3_2);
            CASE(0xee ... 0xef, state_ok, state_tail2);
            // utf8-4
            CASE(0xf0, state_ok, state_utf8_4_1);
            CASE(0xf1 ... 0xf3, state_ok, state_tail3);
            CASE(0xf4, state_ok, state_utf8_4_2);
            // fail
            FAIL(state_ok);
        }
    }
    // utf8-3-1
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0xa0 ... 0xbf, state_utf8_3_1, state_tail1);
            FAIL(state_utf8_3_1);
        }
    }
    // utf8-3-2
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0x9f, state_utf8_3_2, state_tail1);
            FAIL(state_utf8_3_2);
        }
    }
    // utf8-4-1
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x90 ... 0xbf, state_utf8_4_1, state_tail2);
            FAIL(state_utf8_4_1);
        }
    }
    // utf8-4-2
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0x8f, state_utf8_4_2, state_tail2);
            FAIL(state_utf8_4_2);
        }
    }
    // tail1
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0xbf, state_tail1, state_ok);
            FAIL(state_tail1);
        }
    }
    // tail2
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0xbf, state_tail2, state_tail1);
            FAIL(state_tail2);
        }
    }
    // tail3
    for (int i = 0; i < 256; i++) {
        switch (i) {
            CASE(0x80 ... 0xbf, state_tail3, state_tail2);
            FAIL(state_tail3);
        }
    }
    // fail
    for (int i = 0; i < 256; i++) {
        switch (i) {
            FAIL(state_fail);
        }
    }
#undef CASE
#undef FAIL
#undef INSERT
}

uint8_t run4(uint8_t* start, uint8_t* end, enum state state0) {
    uint64_t state = state0 * BITS_PER_STATE2;
    for (uint8_t* s = start; s != end; s++) {
        uint64_t row = table4[*s];
        state = (row >> state) & 63;
    }
    return state / BITS_PER_STATE2;
}

// final shift based dfa

uint8_t run5(uint8_t* start, uint8_t* end, enum state state0) {
    uint64_t state = state0 * BITS_PER_STATE2;
    for (uint8_t* s = start; s != end; s++) {
        uint64_t row = table4[*s];
        state = row >> (state & 63);
    }
    return state / BITS_PER_STATE2;
}

// unroll
uint8_t run6(uint8_t* start, uint8_t* end, enum state state0) {
    uint64_t state = state0 * BITS_PER_STATE2;
    uint8_t* s = start;
    for (; s + 8 < end; s += 8) {
        uint64_t row;
        row = table4[*s];
        state = row >> (state & 63);
        row = table4[*(s + 1)];
        state = row >> (state & 63);
        row = table4[*(s + 2)];
        state = row >> (state & 63);
        row = table4[*(s + 3)];
        state = row >> (state & 63);
        row = table4[*(s + 4)];
        state = row >> (state & 63);
        row = table4[*(s + 5)];
        state = row >> (state & 63);
        row = table4[*(s + 6)];
        state = row >> (state & 63);
        row = table4[*(s + 7)];
        state = row >> (state & 63);
    }

    for (; s != end; s++) {
        uint64_t row = table4[*s];
        state = row >> (state & 63);
    }
    return state / BITS_PER_STATE2;
}
