/* Build options for the ESP32 target, read by faad_internal.h (an Arduino
 * build has no way to pass -D flags). */
#pragma once

/* The I2S path takes mono or stereo; every per-channel buffer scales with this. */
#define MAX_CHANNELS 2

/* SBR and PS (HE-AAC) only where the CPU and PSRAM budget allow it. Without
 * SBR the core is decoded at its own rate, and PS is dropped with it. */
#if !defined(CONFIG_IDF_TARGET_ESP32S3) && !defined(CONFIG_IDF_TARGET_ESP32P4)
#define FAAD_DISABLE_SBR
#endif
