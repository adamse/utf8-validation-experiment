#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <x86intrin.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "validate.h"

char* str_state(enum state state) {
    switch (state) {
        case state_fail: return "fail";
        case state_ok: return "ok";
        case state_tail1: return "tail1";
        case state_tail2: return "tail2";
        case state_tail3: return "tail3";
        case state_utf8_3_1: return "utf8_3_1";
        case state_utf8_3_2: return "utf8_3_2";
        case state_utf8_4_1: return "utf8_4_1";
        case state_utf8_4_2: return "utf8_4_2";
        default: assert(0);
    }
}

int main(int argc, char** argv) {

    if (argc < 2) {
        printf("Usage: %s FILENAME\n", argv[0]);
        exit(1);
    }

    printf("file: %s\n", argv[1]);

    int fd = open(argv[1], O_RDONLY);
    assert(fd != -1);

    struct stat stats = {0};
    int res = fstat(fd, &stats);
    assert(res != -1);
    size_t bytes = stats.st_size;

    void* buf = malloc(bytes);
    assert(buf != NULL);

    ssize_t did_read = read(fd, buf, bytes);
    assert(did_read == bytes);


    fill_table1();
    fill_table2();
    fill_table3();
    fill_table4();

    uint8_t* start_buf = buf;
    uint8_t* end_buf = start_buf + bytes;

#define BENCH(nm, fun) \
    { \
        uint64_t start_time = __rdtsc(); \
        int state = fun(start_buf, end_buf, state_ok); \
        uint64_t time = __rdtsc() - start_time; \
        printf("%-15s: final state=%s, time=%8lu, bytes/time=%f\n", #nm, str_state(state), time, (double)bytes/(double)time); \
    }

    BENCH(naive, run1);
    BENCH(flipped, run2);
    BENCH(shift1, run3);
    // BENCH(shift2, run4);
    BENCH(shift3, run5);
    BENCH(shift3_unroll, run6);

    free(buf);
    assert(-1 != close(fd));

    return 0;
}
