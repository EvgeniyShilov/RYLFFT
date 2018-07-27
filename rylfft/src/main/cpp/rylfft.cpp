#include <jni.h>
#include <cmath>
#include "FftRealPair.hpp"

extern "C"
JNIEXPORT jdouble JNICALL
Java_by_solveit_rylfft_RYLFFT_getSpeechSpectrumEnergyCoefficient(
        JNIEnv *env,
        jclass type,
        jshortArray samples,
        jint count,
        jint sampleRate,
        jdouble minFrequency,
        jdouble maxFrequency
) {
    if (sampleRate <= 0) return -1;
    jshort *samplesArray = env->GetShortArrayElements(samples, NULL);
    std::vector<double> real, imag;
    for (int i = 0; i < count; i++) {
        real.push_back(samplesArray[i]);
        imag.push_back(0);
    }
    Fft::transform(real, imag);
    const double minFrequencyIndex = minFrequency * count / sampleRate;
    const double maxFrequencyIndex = maxFrequency * count / sampleRate;
    double totalEnergy = 0;
    double energyInRange = 0;
    for (int i = 0; i < count / 2; i++) {
        double amplitude = hypot(real[i], imag[i]);
        totalEnergy += amplitude;
        if (i >= minFrequencyIndex && i <= maxFrequencyIndex)
            energyInRange += amplitude;
    }
    const jdouble result = totalEnergy != 0 ? energyInRange / totalEnergy : -1;
    env->ReleaseShortArrayElements(samples, samplesArray, 0);
    return result;
}
