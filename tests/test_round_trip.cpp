/*
 * Round-trip tests: pack an sr_t via write_sr, feed those same bytes back as
 * the SDO response to read_sr, and verify the decoded fields are identical.
 *
 * This confirms that the pack and unpack paths are true inverses for every
 * variant.  Values are kept within each variant's valid data width to avoid
 * deliberate truncation.
 */

#include <gtest/gtest.h>
#include "mock_if.h"

class RoundTripTest : public ::testing::Test {
protected:
    dacx0004_dev_t dev80{}, dev70{}, dev60{};

    void SetUp() override {
        g_mock.reset();
        dacx0004_init_dev(&dev80, DAC80004, &mock_if, nullptr);
        dacx0004_init_dev(&dev70, DAC70004, &mock_if, nullptr);
        dacx0004_init_dev(&dev60, DAC60004, &mock_if, nullptr);
        g_mock.reset();
    }

    // Pack sr via write_sr, use those bytes as the rx_response, then decode
    // with read_sr.  Returns the decoded sr.
    dacx0004_sr_t round_trip(dacx0004_dev_t* dev, dacx0004_sr_t sr) {
        // Pack
        g_mock.reset();
        dacx0004_write_sr(dev, sr);
        if (!g_mock.calls.empty())
            memcpy(g_mock.rx_response, g_mock.calls[0].data, 4);

        // Unpack
        g_mock.calls.clear();
        dacx0004_sr_t result{};
        dacx0004_read_sr(dev, sr, &result);
        return result;
    }

    void expect_fields_equal(dacx0004_sr_t expected, dacx0004_sr_t actual) {
        EXPECT_EQ(actual.Rw,  expected.Rw);
        EXPECT_EQ(actual.cmd, expected.cmd);
        EXPECT_EQ(actual.add, expected.add);
        EXPECT_EQ(actual.dat, expected.dat);
        EXPECT_EQ(actual.mod, expected.mod);
    }
};

// ── DAC80004 (16-bit data) ────────────────────────────────────────────────────

TEST_F(RoundTripTest, DAC80004_Zeros) {
    dacx0004_sr_t sr{};
    expect_fields_equal(sr, round_trip(&dev80, sr));
}

TEST_F(RoundTripTest, DAC80004_MaxData) {
    dacx0004_sr_t sr{};
    sr.Rw = DACX0004_RW_WRITE; sr.cmd = DACX0004_CMD_WRITEn_UPDATEn;
    sr.add = DACX0004_ADD_ALL; sr.dat = 0xFFFF; sr.mod = 0xF;
    expect_fields_equal(sr, round_trip(&dev80, sr));
}

TEST_F(RoundTripTest, DAC80004_KnownValue) {
    dacx0004_sr_t sr{};
    sr.Rw = DACX0004_RW_WRITE; sr.cmd = DACX0004_CMD_WRITEn;
    sr.add = DACX0004_ADD_B; sr.dat = 0x1234; sr.mod = 0x7;
    expect_fields_equal(sr, round_trip(&dev80, sr));
}

TEST_F(RoundTripTest, DAC80004_AlternatingBits) {
    dacx0004_sr_t sr{};
    sr.Rw = DACX0004_RW_READ; sr.cmd = 0x5;
    sr.add = 0xA; sr.dat = 0xAAAA; sr.mod = 0x5;
    expect_fields_equal(sr, round_trip(&dev80, sr));
}

// ── DAC70004 (14-bit data, right-justified) ───────────────────────────────────

TEST_F(RoundTripTest, DAC70004_Zeros) {
    dacx0004_sr_t sr{};
    expect_fields_equal(sr, round_trip(&dev70, sr));
}

TEST_F(RoundTripTest, DAC70004_MaxData) {
    // 14-bit max is 0x3FFF
    dacx0004_sr_t sr{};
    sr.Rw = DACX0004_RW_WRITE; sr.cmd = DACX0004_CMD_WRITEn_UPDATEn;
    sr.add = DACX0004_ADD_C; sr.dat = 0x3FFF; sr.mod = 0xF;
    expect_fields_equal(sr, round_trip(&dev70, sr));
}

TEST_F(RoundTripTest, DAC70004_AlternatingBits) {
    // 0x2AAA = 0b10_1010_1010_1010 (14 bits)
    dacx0004_sr_t sr{};
    sr.Rw = DACX0004_RW_WRITE; sr.cmd = DACX0004_CMD_WRITEn;
    sr.add = DACX0004_ADD_A; sr.dat = 0x2AAA; sr.mod = 0x5;
    expect_fields_equal(sr, round_trip(&dev70, sr));
}

TEST_F(RoundTripTest, DAC70004_Lsbs) {
    // Verify the two least-significant bits of the 14-bit field survive
    dacx0004_sr_t sr{};
    sr.dat = 0x0001;
    expect_fields_equal(sr, round_trip(&dev70, sr));

    sr.dat = 0x0002;
    expect_fields_equal(sr, round_trip(&dev70, sr));

    sr.dat = 0x0003;
    expect_fields_equal(sr, round_trip(&dev70, sr));
}

TEST_F(RoundTripTest, DAC70004_Msbs) {
    // Verify the two most-significant bits of the 14-bit field survive
    dacx0004_sr_t sr{};
    sr.dat = 0x2000;
    expect_fields_equal(sr, round_trip(&dev70, sr));

    sr.dat = 0x1000;
    expect_fields_equal(sr, round_trip(&dev70, sr));
}

// ── DAC60004 (12-bit data, right-justified) ───────────────────────────────────

TEST_F(RoundTripTest, DAC60004_Zeros) {
    dacx0004_sr_t sr{};
    expect_fields_equal(sr, round_trip(&dev60, sr));
}

TEST_F(RoundTripTest, DAC60004_MaxData) {
    // 12-bit max is 0x0FFF
    dacx0004_sr_t sr{};
    sr.Rw = DACX0004_RW_WRITE; sr.cmd = DACX0004_CMD_WRITEn_UPDATEn;
    sr.add = DACX0004_ADD_D; sr.dat = 0x0FFF; sr.mod = 0xF;
    expect_fields_equal(sr, round_trip(&dev60, sr));
}

TEST_F(RoundTripTest, DAC60004_AlternatingBits) {
    // 0x0A5A = 0b1010_0101_1010 (12 bits)
    dacx0004_sr_t sr{};
    sr.Rw = DACX0004_RW_WRITE; sr.cmd = DACX0004_CMD_WRITEn;
    sr.add = DACX0004_ADD_A; sr.dat = 0x0A5A; sr.mod = 0xA;
    expect_fields_equal(sr, round_trip(&dev60, sr));
}

TEST_F(RoundTripTest, DAC60004_ByteBoundary) {
    // Verify that the split between byte1 (high nibble) and byte2 (low byte) is correct
    dacx0004_sr_t sr{};
    sr.dat = 0x0100;  // single bit straddling the byte boundary
    expect_fields_equal(sr, round_trip(&dev60, sr));
}
