/*
 * Verify that ldac_pulse and clr_pulse drive their pins in the correct
 * active-low sequence:
 *
 *   assert   (false → pin LOW, active)
 *   deassert (true  → pin HIGH, inactive)
 *
 * Both LDAC and CLR are active-low.  A "pulse" must assert then deassert —
 * not the other way around — otherwise the pin is left asserted indefinitely.
 *
 * Also verifies that a callback failure on the first call aborts before the
 * second call.
 */

#include <gtest/gtest.h>
#include "mock_if.h"

class GpioOrderTest : public ::testing::Test {
protected:
    dacx0004_dev_t dev{};

    void SetUp() override {
        g_mock.reset();
        dacx0004_init_dev(&dev, DAC80004, &mock_if, nullptr);
        g_mock.reset();
    }
};

// ── dacx0004_ldac_pulse ───────────────────────────────────────────────────────

TEST_F(GpioOrderTest, LdacPulse_MakesTwoCalls) {
    ASSERT_EQ(dacx0004_ldac_pulse(&dev), DACX0004_STAT_OK);
    ASSERT_EQ(g_mock.calls.size(), 2u);
}

TEST_F(GpioOrderTest, LdacPulse_BothCallsTargetLdac) {
    dacx0004_ldac_pulse(&dev);
    for (auto& c : g_mock.calls) {
        EXPECT_EQ(c.fn, MockFn::SET_LDAC);
    }
}

TEST_F(GpioOrderTest, LdacPulse_AssertLowFirst) {
    // First call must drive pin LOW (active-low assert)
    dacx0004_ldac_pulse(&dev);
    EXPECT_EQ(g_mock.calls[0].level, false);
}

TEST_F(GpioOrderTest, LdacPulse_DeassertHighSecond) {
    // Second call must drive pin HIGH (active-low deassert)
    dacx0004_ldac_pulse(&dev);
    EXPECT_EQ(g_mock.calls[1].level, true);
}

TEST_F(GpioOrderTest, LdacPulse_AbortsOnFirstCallFailure) {
    g_mock.next_status = DACX0004_STAT_ERR;
    EXPECT_NE(dacx0004_ldac_pulse(&dev), DACX0004_STAT_OK);
    // Must not attempt the second (deassert) call after the first fails
    EXPECT_EQ(g_mock.calls.size(), 1u);
}

// ── dacx0004_clr_pulse ────────────────────────────────────────────────────────

TEST_F(GpioOrderTest, ClrPulse_MakesTwoCalls) {
    ASSERT_EQ(dacx0004_clr_pulse(&dev), DACX0004_STAT_OK);
    ASSERT_EQ(g_mock.calls.size(), 2u);
}

TEST_F(GpioOrderTest, ClrPulse_BothCallsTargetClr) {
    dacx0004_clr_pulse(&dev);
    for (auto& c : g_mock.calls) {
        EXPECT_EQ(c.fn, MockFn::SET_CLR);
    }
}

TEST_F(GpioOrderTest, ClrPulse_AssertLowFirst) {
    dacx0004_clr_pulse(&dev);
    EXPECT_EQ(g_mock.calls[0].level, false);
}

TEST_F(GpioOrderTest, ClrPulse_DeassertHighSecond) {
    dacx0004_clr_pulse(&dev);
    EXPECT_EQ(g_mock.calls[1].level, true);
}

TEST_F(GpioOrderTest, ClrPulse_AbortsOnFirstCallFailure) {
    g_mock.next_status = DACX0004_STAT_ERR;
    EXPECT_NE(dacx0004_clr_pulse(&dev), DACX0004_STAT_OK);
    EXPECT_EQ(g_mock.calls.size(), 1u);
}
