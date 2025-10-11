// cpuhog.c
#include <stdio.h>
#include <math.h>

int main(void) {
    volatile double x = 0.0;
    for (;;){
        // 忙迴圈吃 CPU
        x += sin(x) * cos(x);
        if (x > 1e9) x = 0.0;
    }
    return 0;
}

