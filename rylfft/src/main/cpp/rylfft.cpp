#include <jni.h>
#include <cmath>
#include "FftRealPair.hpp"

static const int MAX_SHORT_VALUE = 32768;
static const double MAX_DOUBLE_VALUE = 0x1.fffffffffffffP+1023;

static const char *NAME_FRAME_SIZE = "frameSize";
static const char *NAME_ENVELOPE_GROWTH_FACTOR = "envelopeGrowthFactor";
static const char *NAME_ENVELOPE_RECESSION_FACTOR = "envelopeRecessionFactor";
static const char *NAME_STATIC_MULTIPLIER = "staticMultiplier";
static const char *NAME_ENVELOPE_VALUE = "envelopeValue";

static const char *SIG_FRAME_SIZE = "I";
static const char *SIG_ENVELOPE_GROWTH_FACTOR = "D";
static const char *SIG_ENVELOPE_RECESSION_FACTOR = "D";
static const char *SIG_STATIC_MULTIPLIER = "D";
static const char *SIG_ENVELOPE_VALUE = "D";

static jfieldID idFrameSize = NULL;
static jfieldID idEnvelopeGrowthFactor = NULL;
static jfieldID idEnvelopeRecessionFactor = NULL;
static jfieldID idStaticMultiplier = NULL;
static jfieldID idEnvelopeValue = NULL;

static jclass classGainControl = NULL;

jint getFrameSize(JNIEnv *env, jobject instance);

jdouble getEnvelopeGrowthFactor(JNIEnv *env, jobject instance);

jdouble getEnvelopeRecessionFactor(JNIEnv *env, jobject instance);

jdouble getStaticMultiplier(JNIEnv *env, jobject instance);

jdouble getEnvelopeValue(JNIEnv *env, jobject instance);

void setEnvelopeValue(JNIEnv *env, jobject instance, jdouble value);

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

extern "C"
JNIEXPORT void JNICALL
Java_by_solveit_rylfft_RYLGainControl_controlGain(JNIEnv *env,
                                                  jobject instance,
                                                  jbyteArray src,
                                                  jint bytesCount,
                                                  jboolean bigEndian) {
    jbyte *srcBuffer = env->GetByteArrayElements(src, NULL);
    const int sampleCount = bytesCount / 2;
    double sampleBuffer[sampleCount];
    if (bigEndian)
        for (int i = 0; i < sampleCount; i++)
            sampleBuffer[i] =
                    (short) (((srcBuffer[2 * i] & 0xFF) << 8) | (srcBuffer[2 * i + 1] & 0xFF)) /
                    (double) MAX_SHORT_VALUE;
    else
        for (int i = 0; i < sampleCount; i++)
            sampleBuffer[i] =
                    (short) (((srcBuffer[2 * i + 1] & 0xFF) << 8) | (srcBuffer[2 * i] & 0xFF)) /
                    (double) MAX_SHORT_VALUE;
    const jint frameSize = getFrameSize(env, instance);
    const jdouble growthFactor = getEnvelopeGrowthFactor(env, instance);
    const jdouble recessionFactor = getEnvelopeRecessionFactor(env, instance);
    const jdouble staticMultiplier = getStaticMultiplier(env, instance);
    for (int i = 0; i < sampleCount; i += frameSize) {
        double level = 0;
        for (int j = i; j < i + frameSize && j < sampleCount; j++)
            if (sampleBuffer[j] > level) level = sampleBuffer[j];
        const jdouble prevValue = getEnvelopeValue(env, instance);
        const double error = level - prevValue;
        const double delta = (error > 0 ? growthFactor : recessionFactor) * error;
        const jdouble currentValue = prevValue + delta;
        setEnvelopeValue(env, instance, currentValue);
        const double gainMultiplier = currentValue != 0 ? 1 / currentValue : MAX_DOUBLE_VALUE;
        for (int j = i; j < i + frameSize && j < sampleCount; j++) {
            sampleBuffer[j] *= gainMultiplier * staticMultiplier;
            if (sampleBuffer[j] > 1) sampleBuffer[j] = 1;
            else if (sampleBuffer[j] < -1) sampleBuffer[j] = -1;
        }
    }
    if (bigEndian)
        for (int i = 0; i < sampleCount; i++) {
            const short value = (short) (sampleBuffer[i] * MAX_SHORT_VALUE);
            srcBuffer[2 * i] = (jbyte) (value >> 8);
            srcBuffer[2 * i + 1] = (jbyte) value;
        }
    else
        for (int i = 0; i < sampleCount; i++) {
            const short value = (short) (sampleBuffer[i] * MAX_SHORT_VALUE);
            srcBuffer[2 * i + 1] = (jbyte) (value >> 8);
            srcBuffer[2 * i] = (jbyte) value;
        }
    env->ReleaseByteArrayElements(src, srcBuffer, 0);
}

jclass getGainControlClass(JNIEnv *env, jobject instance) {
    if (classGainControl == NULL) classGainControl = env->GetObjectClass(instance);
    return classGainControl;
}

jint getFrameSize(JNIEnv *env, jobject instance) {
    if (idFrameSize == NULL)
        idFrameSize = env->GetFieldID(
                getGainControlClass(env, instance),
                NAME_FRAME_SIZE,
                SIG_FRAME_SIZE
        );
    return env->GetIntField(instance, idFrameSize);
}

jdouble getEnvelopeGrowthFactor(JNIEnv *env, jobject instance) {
    if (idEnvelopeGrowthFactor == NULL)
        idEnvelopeGrowthFactor = env->GetFieldID(
                getGainControlClass(env, instance),
                NAME_ENVELOPE_GROWTH_FACTOR,
                SIG_ENVELOPE_GROWTH_FACTOR
        );
    return env->GetDoubleField(instance, idEnvelopeGrowthFactor);
}

jdouble getEnvelopeRecessionFactor(JNIEnv *env, jobject instance) {
    if (idEnvelopeRecessionFactor == NULL)
        idEnvelopeRecessionFactor = env->GetFieldID(
                getGainControlClass(env, instance),
                NAME_ENVELOPE_RECESSION_FACTOR,
                SIG_ENVELOPE_RECESSION_FACTOR
        );
    return env->GetDoubleField(instance, idEnvelopeRecessionFactor);
}

jdouble getStaticMultiplier(JNIEnv *env, jobject instance) {
    if (idStaticMultiplier == NULL)
        idStaticMultiplier = env->GetFieldID(
                getGainControlClass(env, instance),
                NAME_STATIC_MULTIPLIER,
                SIG_STATIC_MULTIPLIER
        );
    return env->GetDoubleField(instance, idStaticMultiplier);
}

jdouble getEnvelopeValue(JNIEnv *env, jobject instance) {
    if (idEnvelopeValue == NULL)
        idEnvelopeValue = env->GetFieldID(
                getGainControlClass(env, instance),
                NAME_ENVELOPE_VALUE,
                SIG_ENVELOPE_VALUE
        );
    return env->GetDoubleField(instance, idEnvelopeValue);
}

void setEnvelopeValue(JNIEnv *env, jobject instance, jdouble value) {
    if (idEnvelopeValue == NULL)
        idEnvelopeValue = env->GetFieldID(
                getGainControlClass(env, instance),
                NAME_ENVELOPE_VALUE,
                SIG_ENVELOPE_VALUE
        );
    env->SetDoubleField(instance, idEnvelopeValue, value);
}
