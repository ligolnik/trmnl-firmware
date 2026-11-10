#include <panel_vcom.h>
#include <string.h>
#include <unity.h>

static uint8_t otp[PANEL_OTP_VCOM_READ_BYTES];

// A bank's check code and the first bytes of its first temperature range, as read from a TRMNL OG's panel
static void program_bank(size_t bank, uint8_t vcom) {
  otp[bank] = 0xA5;
  const uint8_t range[6] = {0x6f, 0xff, 0x3f, 0x03, 0x00, vcom};
  memcpy(otp + bank + 0x49, range, sizeof(range));
}

void setUp(void) { memset(otp, 0xFF, sizeof(otp)); }

void tearDown(void) {}

void test_reads_vcom_from_the_second_bank(void) {
  program_bank(0xC00, 0x1E);
  TEST_ASSERT_EQUAL_INT(0x1E, panelVcomFromOtp(otp, sizeof(otp)));
}

void test_prefers_the_first_bank(void) {
  program_bank(0x000, 0x1C);
  program_bank(0xC00, 0x1E);
  TEST_ASSERT_EQUAL_INT(0x1C, panelVcomFromOtp(otp, sizeof(otp)));
}

void test_unprogrammed_otp_has_no_vcom(void) { TEST_ASSERT_EQUAL_INT(-1, panelVcomFromOtp(otp, sizeof(otp))); }

void test_bank_without_a_programmed_vcom_has_none(void) {
  program_bank(0xC00, 0xFF);
  TEST_ASSERT_EQUAL_INT(-1, panelVcomFromOtp(otp, sizeof(otp)));
}

void test_too_few_bytes_has_no_vcom(void) {
  program_bank(0xC00, 0x1E);
  TEST_ASSERT_EQUAL_INT(-1, panelVcomFromOtp(otp, 0xC4E));
  TEST_ASSERT_EQUAL_INT(-1, panelVcomFromOtp(nullptr, sizeof(otp)));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_reads_vcom_from_the_second_bank);
  RUN_TEST(test_prefers_the_first_bank);
  RUN_TEST(test_unprogrammed_otp_has_no_vcom);
  RUN_TEST(test_bank_without_a_programmed_vcom_has_none);
  RUN_TEST(test_too_few_bytes_has_no_vcom);
  return UNITY_END();
}
