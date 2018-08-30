package by.solveit.rylfft;

public class RYLGainControl {

    static {
        LibraryLoader.load();
    }

    private int frameSize;
    private double envelopeGrowthFactor;
    private double envelopeRecessionFactor;
    private double staticMultiplier;
    private double envelopeValue;

    public RYLGainControl(int frameSize,
                          double envelopeGrowthFactor,
                          double envelopeRecessionFactor,
                          double staticMultiplier) {
        if (frameSize < 1) throw new RuntimeException("frameSize < 1");
        this.frameSize = frameSize;
        this.envelopeGrowthFactor = envelopeGrowthFactor;
        this.envelopeRecessionFactor = envelopeRecessionFactor;
        this.staticMultiplier = staticMultiplier;
    }

    public native void controlGain(byte[] src, int bytesCount, boolean bigEndian);
}
