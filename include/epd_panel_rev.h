#pragma once

#include <Arduino.h>

// Reads the panel ID (REV command) over bit-banged SPI; 0 when unknown or not readable (bb_epaper boards only)
uint32_t get_panel_rev(void);

// The VCOM_DC code (VDCS 0x82) the factory programmed into the panel's OTP, read once per panel and cached;
// -1 when unknown. Call right after get_panel_rev(), before bb_epaper takes over the SPI pins.
int get_panel_vcom(uint32_t panel_id);

/**
 * @brief Panel ID (EPD revision) as 8 hex digits for the Panel-Rev request header
 * @return hex string, or empty if the ID is unknown/not read
 */
String display_panel_rev_string(void);
