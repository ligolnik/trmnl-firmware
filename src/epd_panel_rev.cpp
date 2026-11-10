#include <board_config.h>
#include <epd_driver.h>
#include <epd_panel_rev.h>
#include <globals.h>
#include <inttypes.h>
#include <panel_vcom.h>

#ifdef BB_EPAPER
static void WriteSPIByte(uint8_t data) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(pDevice->epd_mosi_pin, (data & 0x80) ? 1 : 0);
    digitalWrite(pDevice->epd_sck_pin, 1);
    digitalWrite(pDevice->epd_sck_pin, 0);
    data <<= 1;
  }
}

static uint8_t ReadSPIByte(void) {
  uint8_t u8 = 0;
  for (int i = 0; i < 8; i++) {
    digitalWrite(pDevice->epd_sck_pin, 1);
    digitalWrite(pDevice->epd_sck_pin, 0);
    u8 <<= 1;
    u8 |= digitalRead(pDevice->epd_mosi_pin);
  }
  return u8;
}
/**
 * @brief Function to read the EPD revision to know what temperature profile to use
 * @param none
 * @return 32-bit value read from the panel's revision (REV) command
 */
uint32_t get_panel_rev(void) {
  uint32_t u32 = 0;
  uint8_t u8;

  if (pDevice->epd_mosi_pin == 0 && pDevice->epd_sck_pin == 0) { // pre-defined PCB+display in bb_epaper; pins unknown
    return 0;
  }
  if (pDevice->panel_set != EPD_75) { // REV (0x70) is only known to be a harmless read on the 7.5" B/W UC8179
    return 0;
  }

    // Initialize the SPI bus in 'bit-bang' mode before running the normal init sequence
    // This will allow us to use MOSI as a bidirectional line for reading data from the panel
  pinMode(pDevice->epd_sck_pin, OUTPUT);
  pinMode(pDevice->epd_cs_pin, OUTPUT);
  digitalWrite(pDevice->epd_cs_pin, 1);
  pinMode(pDevice->epd_rst_pin, OUTPUT);
  pinMode(pDevice->epd_dc_pin, OUTPUT);
  pinMode(pDevice->epd_busy_pin, INPUT);

    // Reset the panel
  digitalWrite(pDevice->epd_rst_pin, 0);
  delay(20);
  digitalWrite(pDevice->epd_rst_pin, 1);
  delay(20);
  if (digitalRead(pDevice->epd_busy_pin) ==
      0) { // it's a SSD16xx; not useful at the moment because our 7.5" B/W is a UC81xx type panel
    return 0;
  }
  digitalWrite(pDevice->epd_dc_pin, 0); // command mode
  digitalWrite(pDevice->epd_cs_pin, 0);
  pinMode(pDevice->epd_mosi_pin, OUTPUT);
  WriteSPIByte(0x70); // Revision (REV) command
  digitalWrite(pDevice->epd_dc_pin, 1); // data mode
  pinMode(pDevice->epd_mosi_pin, INPUT);
  for (int i = 0; i < 7; i++) {
    u8 = ReadSPIByte();
    if (i >= 3) { // first 3 bytes are 0xff
      u32 <<= 8;
      u32 |= u8;
    }
  }
  digitalWrite(pDevice->epd_cs_pin, 1);
  return u32;
} /* get_panel_rev() */

// Samples while SCK is high, as the UC8179 read timing has it; ReadSPIByte() would drop each byte's MSB
static uint8_t read_otp_byte(void) {
  uint8_t u8 = 0;
  for (int i = 0; i < 8; i++) {
    delayMicroseconds(2);
    digitalWrite(pDevice->epd_sck_pin, 1);
    delayMicroseconds(2);
    u8 = (u8 << 1) | digitalRead(pDevice->epd_mosi_pin);
    digitalWrite(pDevice->epd_sck_pin, 0);
  }
  return u8;
}

static int read_panel_vcom(void) {
  uint8_t *otp = (uint8_t *)malloc(PANEL_OTP_VCOM_READ_BYTES);
  if (!otp) return -1;
  digitalWrite(pDevice->epd_dc_pin, 0); // command mode
  digitalWrite(pDevice->epd_cs_pin, 0);
  pinMode(pDevice->epd_mosi_pin, OUTPUT);
  WriteSPIByte(0xA2); // Read OTP (ROTP) command, from address 0
  digitalWrite(pDevice->epd_dc_pin, 1); // data mode
  pinMode(pDevice->epd_mosi_pin, INPUT);
  for (size_t i = 0; i < PANEL_OTP_VCOM_READ_BYTES; i++)
    otp[i] = read_otp_byte();
  digitalWrite(pDevice->epd_cs_pin, 1);
  const int vcom = panelVcomFromOtp(otp, PANEL_OTP_VCOM_READ_BYTES);
  free(otp);
  return vcom;
}

int get_panel_vcom(uint32_t panel_id) {
  if (panel_id == 0) return -1; // not a panel get_panel_rev() could read
  if (preferences.getUInt(PREFERENCES_PANEL_VCOM_ID, 0) == panel_id && preferences.isKey(PREFERENCES_PANEL_VCOM)) {
    return preferences.getInt(PREFERENCES_PANEL_VCOM, -1);
  }
  const int vcom = read_panel_vcom();
  preferences.putInt(PREFERENCES_PANEL_VCOM, vcom);
  preferences.putUInt(PREFERENCES_PANEL_VCOM_ID, panel_id);
  return vcom;
}
#endif // BB_EPAPER

String display_panel_rev_string(void) {
  if (panel_rev == 0) {
    return String();
  }
  char sz[9];
  snprintf(sz, sizeof(sz), "%08" PRIx32, panel_rev);
  return String(sz);
} /* display_panel_rev_string() */
