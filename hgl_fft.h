/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2024 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_fft.h implements the discrete Fast Fourier Transform and Inverese Fast Fourier
 * Transform.
 *
 *
 * USAGE:
 *
 * Include hgl_fft.h file like this:
 *
 *     #define HGL_FFT_IMPLEMENTATION
 *     #include "hgl_fft.h"
 *
 * HGL_FFT_IMPLEMENTATION must only be defined once, in a single compilation unit.
 *
 * Optionally, you can define:
 *
 *     #define HGL_FFT_USE_SIMD
 *
 * HGL_FFT_USE_SIMD will utilize SSE2 SIMD instructions to speed up the implementation.
 *
 * EXAMPLE:
 *
 *     #define HGL_FFT_IMPLEMENTATION
 *     #define HGL_FFT_USE_SIMD
 *     #include "hgl_fft.h"
 *
 *     #define N (1 << 20)
 *
 *     float signal[N];
 *     float complex signal_frequencies[N];
 *     float complex reconstructed_signal[N];
 *
 *     int main(void)
 *     {
 *         // Generate some signal
 *         for (int i = 0; i < N; i++) {
 *             float t = 1.0f * (float)i/N;
 *             signal[i] = sinf(1*2*PI*t) + sinf(2*2*PI*t) + cosf(3*2*PI*t);
 *         }
 *
 *         // Normalize (not necessary)
 *         float max = 0;
 *         for (int i = 0; i < N; i++) {
 *             max = (fabsf(signal[i]) > max) ? fabsf(signal[i]) : max;
 *         }
 *         for (int i = 0; i < N; i++) {
 *             signal[i] /= max;
 *         }
 *
 *         // Perform Fourier Transform on `signal`
 *         fft(signal, signal_frequencies, N);
 *
 *         // Do something with frequency domain data
 *         float cutoff_freq_hz = 120;
 *         high_pass_filter(signal_frequencies, cutoff_freq_hz, N);
 *
 *         // Reconstruct signal from frequencies.
 *         ifft(signal_frequencies, reconstructed_signal, N);
 *     }
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_FFT_H
#define HGL_FFT_H

#include <complex.h>

/*--- Public function prototypes --------------------------------------------------------*/
void fft(float in[], float complex out[], int n);
void ifft(float complex in[], float complex out[], int n);

#endif /* HGL_FFT_H */

#ifdef HGL_FFT_IMPLEMENTATION

#include <math.h>
#include <assert.h>
#ifdef HGL_FFT_USE_SIMD
#   include <smmintrin.h>
#   include <immintrin.h>
#endif

#ifndef PI
#define PI 3.14159265358979
#endif

void fft_(float in[], float complex out[], int n, int stride);
void ifft_(float complex in[], float complex out[], int n, int stride);

void fft_(float in[], float complex out[], int n, int stride)
{
    if (n == 1) {
        out[0] = in[0];
        return;
    }

    fft_(in, out, n/2, 2*stride); // even
    fft_(in + stride, out + n/2, n/2, 2*stride); // odd

#ifndef HGL_FFT_USE_SIMD
    for(int k = 0; k < n/2; k++) {
        float w = -2*PI*((float)k/n);
        float complex v = out[k + n/2] * (cosf(w) + I*sinf(w));
        out[k + n/2]    = out[k] - v;
        out[k]          = out[k] + v;
    }
#else
    if (n/2 < 4) {
        for(int k = 0; k < n/2; k++) {
            float w = -2*PI*((float)k/n);
            float complex v = out[k + n/2] * (cosf(w) + I*sinf(w));
            //float complex v = out[k + n/2] * cexpf(-2*PI*((float)k/n)*I);
            out[k + n/2]    = out[k] - v;
            out[k]          = out[k] + v;
        }
    } else {
        for(int k = 0; k < n/2; k += 4) {
            float ws[3]  __attribute__((aligned(16)));
            __m128 vec_ks      = _mm_set_ps1((float)(k));          // (k, k, k, k)
            __m128 vec_rns     = _mm_set_ps1(1.0f/n);              // (1/n, 1/n, 1/n, 1/n)
            __m128 vec_2pi     = _mm_set_ps1(-2*PI);             // (2*PI, 2*PI, 2*PI, 2*PI)
            __m128 vec_offsets = _mm_set_ps(3, 2, 1, 0);           // (0, 1, 2, 3)
            __m128 vec_indices = _mm_add_ps(vec_ks, vec_offsets);  // (k, k+1, k+2, k+3)
            __m128 vec_ts      = _mm_mul_ps(vec_indices, vec_rns); // (k/n, (k+1)/n, (k+2)/n, (k+3)/n)
            __m128 vec_ws      = _mm_mul_ps(vec_ts, vec_2pi);      // (2*PI*(k/n), 2*PI*((k+1)/n), 2*PI*((k+2)/n), 2*PI*((k+3)/n))
            _mm_store_ps(ws, vec_ws);
            float complex v0 = out[k + n/2] * (cosf(ws[0]) + I*sinf(ws[0]));
            float complex v1 = out[k + n/2 + 1] * (cosf(ws[1]) + I*sinf(ws[1]));
            float complex v2 = out[k + n/2 + 2] * (cosf(ws[2]) + I*sinf(ws[2]));
            float complex v3 = out[k + n/2 + 3] * (cosf(ws[3]) + I*sinf(ws[3]));
            out[k + n/2]     = out[k] - v0;
            out[k]           = out[k] + v0;
            out[k + n/2 + 1] = out[k + 1] - v1;
            out[k + 1]       = out[k + 1] + v1;
            out[k + n/2 + 2] = out[k + 2] - v2;
            out[k + 2]       = out[k + 2] + v2;
            out[k + n/2 + 3] = out[k + 3] - v3;
            out[k + 3]       = out[k + 3] + v3;
        }
    }
#endif

}

void ifft_(float complex in[], float complex out[], int n, int stride)
{
    if (n == 1) {
        out[0] = in[0];
        return;
    }

    ifft_(in, out, n/2, 2*stride); // even
    ifft_(in + stride, out + n/2, n/2, 2*stride); // odd

#ifndef HGL_FFT_USE_SIMD
    for(int k = 0; k < n/2; k++) {
        float w = 2*PI*((float)k/n);
        float complex v = out[k + n/2] * (cosf(w) + I*sinf(w));
        out[k + n/2]    = out[k] - v;
        out[k]          = out[k] + v;
    }
#else
    if (n/2 < 4) {
        for(int k = 0; k < n/2; k++) {
            float w = 2*PI*((float)k/n);
            float complex v = out[k + n/2] * (cosf(w) + I*sinf(w));
            out[k + n/2]    = out[k] - v;
            out[k]          = out[k] + v;
        }
    } else {
        for(int k = 0; k < n/2; k += 4) {
            float ws[3]  __attribute__((aligned(16)));
            __m128 vec_ks      = _mm_set_ps1((float)(k));          // (k, k, k, k)
            __m128 vec_rns     = _mm_set_ps1(1.0f/n);              // (1/n, 1/n, 1/n, 1/n)
            __m128 vec_2pi     = _mm_set_ps1(2*PI);              // (2*PI, 2*PI, 2*PI, 2*PI)
            __m128 vec_offsets = _mm_set_ps(3, 2, 1, 0);           // (0, 1, 2, 3)
            __m128 vec_indices = _mm_add_ps(vec_ks, vec_offsets);  // (k, k+1, k+2, k+3)
            __m128 vec_ts      = _mm_mul_ps(vec_indices, vec_rns); // (k/n, (k+1)/n, (k+2)/n, (k+3)/n)
            __m128 vec_ws      = _mm_mul_ps(vec_ts, vec_2pi);      // (2*PI*(k/n), 2*PI*((k+1)/n), 2*PI*((k+2)/n), 2*PI*((k+3)/n))
            _mm_store_ps(ws, vec_ws);
            float complex v0 = out[k + n/2] * (cosf(ws[0]) + I*sinf(ws[0]));
            float complex v1 = out[k + n/2 + 1] * (cosf(ws[1]) + I*sinf(ws[1]));
            float complex v2 = out[k + n/2 + 2] * (cosf(ws[2]) + I*sinf(ws[2]));
            float complex v3 = out[k + n/2 + 3] * (cosf(ws[3]) + I*sinf(ws[3]));
            out[k + n/2]        = out[k] - v0;
            out[k]              = out[k] + v0;
            out[k + n/2 + 1]    = out[k + 1] - v1;
            out[k + 1]          = out[k + 1] + v1;
            out[k + n/2 + 2]    = out[k + 2] - v2;
            out[k + 2]          = out[k + 2] + v2;
            out[k + n/2 + 3]    = out[k + 3] - v3;
            out[k + 3]          = out[k + 3] + v3;
        }
    }
#endif

}

void fft(float in[], float complex out[], int n)
{
    assert((n & (n - 1)) == 0); // n is power of 2
    fft_(in, out, n, 1);
}

void ifft(float complex in[], float complex out[], int n)
{
    assert((n & (n - 1)) == 0); // n is power of 2
    ifft_(in, out, n, 1);
    for (int i = 0; i < n; i++) {
        out[i] = out[i]/n;
    }
}

#endif /* HGL_FFT_IMPLEMENTATION */
