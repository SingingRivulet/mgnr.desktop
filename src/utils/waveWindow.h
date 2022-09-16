#pragma once

#include <math.h>

namespace sinrivUtils::waveWindow {

typedef enum {
    SOUND_WINDOW_HANNING = 0,
    SOUND_WINDOW_HAMMING = 1,
    SOUND_WINDOW_TRIANGULAR = 2,
    SOUND_WINDOW_GAUSS = 3,
    SOUND_WINDOW_BLACKMAN_HARRIS = 4,
    SOUND_WINDOW_FLAT = 5,
    SOUND_WINDOW_RANDOM = 6,
    SOUND_WINDOW_ALMOST_FLAT = 7
} SoundWindowType;

inline float hanningAtPoint(float point, int numSamples) {
    return 0.5f * (1.0f - cosf((2.0f * M_PI * point) / ((float)numSamples - 1.0f)));
}

inline float hammingAtPoint(int point, int numSamples) {
    return 0.54f - 0.46f * cosf((2.0f * M_PI * point) / ((float)numSamples - 1.0f));
}

inline float triangularAtPoint(int point, int numSamples) {
    return (2.0f / numSamples) * ((numSamples * 0.5f) - fabs(point - (numSamples - 1.0f) * 0.5f));
}

inline float gaussAtPoint(int point, int numSamples) {
    float bellW = 0.4f;
    return powf(M_E, -0.5f * powf((point - (numSamples - 1) * 0.5f) / (bellW * (numSamples - 1) * 0.5f), 2.0f));
}

inline float blackmanHarrisAtPoint(int point, int numSamples) {
    return 0.35875f - 0.48829f * cosf(2.0f * M_PI * point / (numSamples - 1)) + 0.14128f * cosf(4.0f * M_PI * point / (numSamples - 1)) - 0.01168f * cosf(6.0f * M_PI * point / (numSamples - 1));
}

inline float randomAtPoint(int point, int numSamples) {
    return rand() % 1000 / 1000.0f;
}

inline void create(float* soundWindow, int length, SoundWindowType windowType) {
    switch (windowType) {
        case SOUND_WINDOW_HANNING:
            for (int i = 0; i < length; i++)
                soundWindow[i] = hanningAtPoint(i, length);
            break;

        case SOUND_WINDOW_HAMMING:
            for (int i = 0; i < length; i++)
                soundWindow[i] = hammingAtPoint(i, length);
            break;

        case SOUND_WINDOW_TRIANGULAR:
            for (int i = 0; i < length; i++)
                soundWindow[i] = triangularAtPoint(i, length);
            break;

        case SOUND_WINDOW_GAUSS:
            for (int i = 0; i < length; i++)
                soundWindow[i] = gaussAtPoint(i, length);
            break;

        case SOUND_WINDOW_BLACKMAN_HARRIS:
            for (int i = 0; i < length; i++)
                soundWindow[i] = blackmanHarrisAtPoint(i, length);
            break;

        case SOUND_WINDOW_FLAT:
            for (int i = 0; i < length; i++)
                soundWindow[i] = 1.0f;
            break;

        case SOUND_WINDOW_RANDOM:
            for (int i = 0; i < length; i++)
                soundWindow[i] = randomAtPoint(i, length);
            break;

        case SOUND_WINDOW_ALMOST_FLAT:
            for (int i = 0; i < length; i++) {
                soundWindow[i] = powf(2 * hanningAtPoint(i, length), 0.3);
                soundWindow[i] = powf(soundWindow[i], 1.5);
                if (soundWindow[i] > 1.0f)
                    soundWindow[i] = 1.0f;
            }
            break;
            ;
        default:
            for (int i = 0; i < length; i++)
                soundWindow[i] = 1.0f;
            printf("Undefined SoundWindowType!\n");
            break;
    }
}

}  // namespace sinrivUtils::waveWindow