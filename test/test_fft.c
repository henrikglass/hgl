
#include "hgl_test.h"

#define HGL_FFT_IMPLEMENTATION
#include "hgl_fft.h"

#define N 128
float signal_sample[N];
float complex signal_sample_frequencies[N];
float complex reconstructed_signal_sample[N];

bool float_eq(float a, float b)
{
    const float EPSILON = 0.00001f;
    return fabsf(a - b) < EPSILON;
}

void generate_signal()
{
    /* generate a signal with a dominant frequency of 10 */
    for (int i = 0; i < N; i++) {
        float t = 1.0f * (float)i/N;
        signal_sample[i] =        sinf(10*2*PI*t) + 
                           0.2  * sinf(15*2*PI*t) +
                           0.15 * cosf(16*2*PI*t) +
                           0.12 * cosf(2*2*PI*t);
    }

    /* normalize */
    float max = 0;
    for (int i = 0; i < N; i++) {
        max = (fabsf(signal_sample[i]) > max) ? fabsf(signal_sample[i]) : max;
    }
    for (int i = 0; i < N; i++) {
        signal_sample[i] /= max;
    }
}

TEST(test_fft, .setup = generate_signal)
{
    // perform fft 
    fft(signal_sample, signal_sample_frequencies, N); 

    // assert that the dominant frequency is 10
    int idx = -1;
    float max = 0.0f;
    for (int i = 0; i < N/2; i++) {
        if (cabsf(signal_sample_frequencies[i]) > max) {
            max = cabsf(signal_sample_frequencies[i]);
            idx = i;
        }
    }
    ASSERT(idx == 10);

    // assert that the magnitude at freq (N-idx) is the same as at idx
    ASSERT(float_eq(cabsf(signal_sample_frequencies[idx]), 
                    cabsf(signal_sample_frequencies[N - idx])));
}

TEST(test_ifft, .setup = generate_signal)
{
    // perform fft 
    fft(signal_sample, signal_sample_frequencies, N); 

    // perform inverse fft
    ifft(signal_sample_frequencies, reconstructed_signal_sample, N);

    // assert that the reconstructed signal is identical to the orignal signal
    // (accounting for rounding errors)
    for (int i = 0; i < N; i++) {
        ASSERT(float_eq(signal_sample[i], creal(reconstructed_signal_sample[i])));
    }

}
