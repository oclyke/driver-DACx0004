#include <gtest/gtest.h>
#include "mock_if.h"

class NullGuardTest : public ::testing::Test {
protected:
    dacx0004_dev_t dev{};
    dacx0004_sr_t  sr{};

    void SetUp() override {
        g_mock.reset();
        dacx0004_init_dev(&dev, DAC80004, &mock_if, nullptr);
        g_mock.reset();  // discard init-time gpio calls
    }
};

// ── dacx0004_init_dev ────────────────────────────────────────────────────────

TEST_F(NullGuardTest, InitDev_NullDev) {
    EXPECT_EQ(dacx0004_init_dev(nullptr, DAC80004, &mock_if, nullptr),
              DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, InitDev_NullIf) {
    dacx0004_dev_t d{};
    EXPECT_EQ(dacx0004_init_dev(&d, DAC80004, nullptr, nullptr),
              DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, InitDev_BadVersion) {
    dacx0004_dev_t d{};
    EXPECT_EQ(dacx0004_init_dev(&d, DACX0004_VER_NUM, &mock_if, nullptr),
              DACX0004_STAT_ERR_UNKNOWN_VER);
}

// ── dacx0004_write_sr ────────────────────────────────────────────────────────

TEST_F(NullGuardTest, WriteSr_NullDev) {
    EXPECT_EQ(dacx0004_write_sr(nullptr, sr), DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, WriteSr_NullIf) {
    dacx0004_dev_t d{};
    // d._if is nullptr after zero-init
    EXPECT_EQ(dacx0004_write_sr(&d, sr), DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, WriteSr_NullShiftSr) {
    dacx0004_if_t no_shift{};  // all function pointers null
    dacx0004_dev_t d{};
    d._ver = DAC80004;
    d._if  = &no_shift;
    EXPECT_EQ(dacx0004_write_sr(&d, sr), DACX0004_STAT_ERR_INVALID_ARG);
}

// ── dacx0004_read_sr ─────────────────────────────────────────────────────────

TEST_F(NullGuardTest, ReadSr_NullDev) {
    dacx0004_sr_t result{};
    EXPECT_EQ(dacx0004_read_sr(nullptr, sr, &result), DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, ReadSr_NullResult) {
    EXPECT_EQ(dacx0004_read_sr(&dev, sr, nullptr), DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, ReadSr_NullIf) {
    dacx0004_dev_t d{};
    dacx0004_sr_t result{};
    EXPECT_EQ(dacx0004_read_sr(&d, sr, &result), DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, ReadSr_NoRwCallback) {
    dacx0004_dev_t d{};
    dacx0004_init_dev(&d, DAC80004, &mock_if_no_rw, nullptr);
    g_mock.reset();
    dacx0004_sr_t result{};
    EXPECT_EQ(dacx0004_read_sr(&d, sr, &result), DACX0004_STAT_ERR_INVALID_ARG);
}

// ── high-level channel helpers ───────────────────────────────────────────────

TEST_F(NullGuardTest, WriteChannel_NullDev) {
    EXPECT_EQ(dacx0004_write_channel(nullptr, DACX0004_ADD_A, 0),
              DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, UpdateChannel_NullDev) {
    EXPECT_EQ(dacx0004_update_channel(nullptr, DACX0004_ADD_A),
              DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, WriteUpdateChannel_NullDev) {
    EXPECT_EQ(dacx0004_write_update_channel(nullptr, DACX0004_ADD_A, 0),
              DACX0004_STAT_ERR_INVALID_ARG);
}

// ── control helpers ──────────────────────────────────────────────────────────

TEST_F(NullGuardTest, SoftwareReset_NullDev) {
    EXPECT_EQ(dacx0004_software_reset(nullptr), DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, SoftwareClear_NullDev) {
    EXPECT_EQ(dacx0004_software_clear(nullptr), DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, SetPower_NullDev) {
    EXPECT_EQ(dacx0004_set_power(nullptr, DACX0004_ADD_A, DACX0004_PWR_UP),
              DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, SetClearMode_NullDev) {
    EXPECT_EQ(dacx0004_set_clear_mode(nullptr, DACX0004_CLM_ZERO),
              DACX0004_STAT_ERR_INVALID_ARG);
}

// ── GPIO helpers ─────────────────────────────────────────────────────────────

TEST_F(NullGuardTest, LdacPulse_NullDev) {
    EXPECT_EQ(dacx0004_ldac_pulse(nullptr), DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, LdacPulse_NullIf) {
    dacx0004_dev_t d{};
    EXPECT_EQ(dacx0004_ldac_pulse(&d), DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, LdacPulse_NoCallback) {
    dacx0004_dev_t d{};
    dacx0004_init_dev(&d, DAC80004, &mock_if_no_gpio, nullptr);
    g_mock.reset();
    EXPECT_EQ(dacx0004_ldac_pulse(&d), DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, ClrPulse_NullDev) {
    EXPECT_EQ(dacx0004_clr_pulse(nullptr), DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, ClrPulse_NullIf) {
    dacx0004_dev_t d{};
    EXPECT_EQ(dacx0004_clr_pulse(&d), DACX0004_STAT_ERR_INVALID_ARG);
}

TEST_F(NullGuardTest, ClrPulse_NoCallback) {
    dacx0004_dev_t d{};
    dacx0004_init_dev(&d, DAC80004, &mock_if_no_gpio, nullptr);
    g_mock.reset();
    EXPECT_EQ(dacx0004_clr_pulse(&d), DACX0004_STAT_ERR_INVALID_ARG);
}
