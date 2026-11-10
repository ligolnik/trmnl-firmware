#include <panel_vcom.h>

namespace {
  const size_t BANK_ADDRESSES[] = {0x000, 0xC00}; // UC8179 datasheet, OTP address map
  const uint8_t BANK_CHECK_CODE = 0xA5;
  const size_t FIRST_RANGE_VCOM_OFFSET = 0x4E; // 0b, VCOM_DC[6:0]
} // namespace

int panelVcomFromOtp(const uint8_t *otp, size_t size) {
  if (!otp) return -1;
  for (size_t bank : BANK_ADDRESSES) {
    if (bank + FIRST_RANGE_VCOM_OFFSET >= size || otp[bank] != BANK_CHECK_CODE) continue;
    const uint8_t vcom = otp[bank + FIRST_RANGE_VCOM_OFFSET];
    return (vcom & 0x80) ? -1 : vcom; // unprogrammed bits read 1
  }
  return -1;
}
