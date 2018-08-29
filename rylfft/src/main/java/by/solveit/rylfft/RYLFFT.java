package by.solveit.rylfft;

public class RYLFFT {

    static {
        LibraryLoader.load();
    }

    public static native double getSpeechSpectrumEnergyCoefficient(
            short[] samples,
            int count,
            int sampleRate,
            double minFrequency,
            double maxFrequency
    );
}
