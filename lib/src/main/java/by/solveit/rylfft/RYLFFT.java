package by.solveit.rylfft;

public class RYLFFT {

    static {
        System.loadLibrary("rylfft");
    }

    public static native double getSpeechSpectrumEnergy(
            short[] samples,
            int count,
            int sampleRate,
            double minFrequency,
            double maxFrequency
    );
}
