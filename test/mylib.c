#include <stdio.h>

void mylib_print(void);
void mylib_print(void) {
    printf("Hello from mylib! (built: %s %s)\n", __DATE__, __TIME__);
}
