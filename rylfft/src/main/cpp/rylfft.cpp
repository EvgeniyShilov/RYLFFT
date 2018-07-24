#include <jni.h>
#include <cmath>
#include "FftRealPair.hpp"

extern "C"
JNIEXPORT jdouble JNICALL
Java_by_solveit_rylfft_RYLFFT_getSpeechSpectrumEnergy(
        JNIEnv *env,
        jclass type,
        jshortArray samples,
        jint count,
        jint sampleRate,
        jdouble minFrequency,
        jdouble maxFrequency
) {
    jshort *samplesArray = env->GetShortArrayElements(samples, NULL);
    std::vector<double> real, imag;
    for (int i = 0; i < count; i++) {
        real.push_back(samplesArray[i]);
        imag.push_back(0);
    }
    Fft::transform(real, imag);
    const double minFrequencyIndex = minFrequency * count / sampleRate;
    const double maxFrequencyIndex = maxFrequency * count / sampleRate;
    jdouble result = 0;
    for (int i = (int) minFrequencyIndex; i < maxFrequencyIndex; i++)
        result += hypot(real[i], imag[i]);
    result = result * sampleRate / count;
    env->ReleaseShortArrayElements(samples, samplesArray, 0);
    return result;
}
