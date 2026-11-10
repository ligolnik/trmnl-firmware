#pragma once

#include <stddef.h>
#include <stdint.h>

// OTP bytes to read from a UC8179 to find its VCOM: through bank 1's first temperature range
#define PANEL_OTP_VCOM_READ_BYTES 0xC4F

// The VCOM_DC code (VDCS 0x82: -0.10 V - 0.05 V per step) the factory programmed into a UC8179's OTP,
// from its first temperature range; -1 when the OTP holds none
int panelVcomFromOtp(const uint8_t *otp, size_t size);
