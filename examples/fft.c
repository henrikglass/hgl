
#define HGL_FFT_IMPLEMENTATION
#define HGL_FFT_USE_SIMD
#include "hgl_fft.h"

#include <stdio.h>
#include <stdbool.h>

#define N 64

float signal[N];
float complex signal_frequencies[N];

float reconstructed_signal_real[N];
float complex reconstructed_signal[N];

void draw_signal(float in[], int n);
void draw_signal(float in[], int n)
{
    const int S = 20;
    for (int i = 0; i < n; i++) {
        int x = (int)(S*in[i]);
        if (x < 0) {
            for (int j = 0; j < S; j++) if(S + x > j) printf(" "); else printf("#");
            printf("|\n");
        } else {
            printf("%*.s|", S, " ");
            for (int j = 0; j < x; j++) printf("#");
            printf("\n");
        }
    }
}

bool float_equals(float a, float b);
bool float_equals(float a, float b)
{
    return fabsf(b - a) < 0.0001f;
}

int main()
{

    /* ======================== generate some normalized signal ======================= */
    for (int i = 0; i < N; i++) {
        float t = 1.0f * (float)i/N;
        float t2 = 1.0f * (float)(i+7)/N;
        signal[i] = sinf(1*2*PI*t) + sinf(2*2*PI*t2) + cosf(3*2*PI*t);
    }

    // normalize
    float max = 0;
    for (int i = 0; i < N; i++) {
        max = (fabsf(signal[i]) > max) ? fabsf(signal[i]) : max;
    }
    for (int i = 0; i < N; i++) {
        signal[i] /= max;
    }


    printf("Original signal:\n");
    draw_signal(signal, N);

    fft(signal, signal_frequencies, N);

    //signal_frequencies[2] = 0;
    //signal_frequencies[N-2] = 0;

    ifft(signal_frequencies, reconstructed_signal, N);

    printf("Reconstructed signal:\n");

    for (int i = 0; i < N; i++) {
        reconstructed_signal_real[i] = creal(reconstructed_signal[i]);
    }

    draw_signal(reconstructed_signal_real, N);


    bool equal = true;
    for (int i = 0; i < N; i++) {
        equal = equal && float_equals(signal[i], reconstructed_signal_real[i]);
    }

    if (equal) {
        printf("Original & reconstructed signals are equal!\n");
    } else {
        printf("Original & reconstructed signals are NOT equal!\n");
    }

    printf("original frequenciess:\n");
    for (int i = 0; i < N; i++) {
        printf("[%d] %4.2f + %4.2fi\n",
                i,
                creal(signal_frequencies[i]),
                cimag(signal_frequencies[i]));
    }

}
