package by.solveit.rylfft;

public class LibraryLoader {

    private static volatile boolean libWasLoaded;

    public static void load() {
        if (!libWasLoaded) synchronized (LibraryLoader.class) {
            if (!libWasLoaded) {
                System.loadLibrary("rylfft");
                libWasLoaded = true;
            }
        }
    }
}
