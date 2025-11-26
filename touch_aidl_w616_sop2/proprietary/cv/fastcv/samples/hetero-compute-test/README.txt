INFO:
======
Demonstrates heterogeneous computation support with a sample pipeline in FastCV overriding default opMode settings.
 - Configure FastCV to a particular operation mode using fcvSetOperationMode and execute a computation pipeline on different HW subsystems, overriding default configuration of mode settings using fcvSetFunctionApiToCore.
 - Following pipeline is executed fcvScaleDownMNu8 -> fcvFilterGaussian3x3u8_v2 -> fcvCornerHarrisu8.
 - fcvScaleDownMNu8 is executed in default opMode configuration, whereas fcvFilterGaussian3x3u8_v2 and fcvCornerHarrisu8 execution preferences are overridden using fcvSetFunctionApiToCore to configure for GPU and NSP resp.
