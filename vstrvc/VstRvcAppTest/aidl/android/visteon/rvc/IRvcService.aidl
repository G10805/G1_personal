// IRvcService.aidl
package android.visteon.rvc;

interface IRvcService {
    void setClipBounds(int left, int top, int right, int bottom);
    void startPreview();
    void stopPreview();
    boolean hasStarted();
}
