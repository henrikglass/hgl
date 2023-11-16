#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "hgl_neat.h"

int test() {
    TRY(fcntl(2124, 124, 0)); // TRY something that will fail. Should return -1.
    return 0;
}

int main()
{
    f32 fisk = 123.0f;
    printf("fisk = %f\n", (double)fisk);
    printf("fisk bits = 0x%08X\n", TRANSMUTE(u32, fisk));
    assert(test() == -1);
    return 0;
}
