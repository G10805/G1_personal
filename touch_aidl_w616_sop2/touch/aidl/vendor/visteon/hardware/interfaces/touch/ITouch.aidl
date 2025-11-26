package vendor.visteon.hardware.interfaces.touch;

import vendor.visteon.hardware.interfaces.touch.ITouchCallback;

@VintfStability
interface ITouch {
    void registerCallback(ITouchCallback callback, int clientID);
}
