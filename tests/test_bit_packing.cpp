/*
 * Verify that dacx0004_write_sr packs each field into the correct wire bits.
 *
 * Wire frame (32 bits, sent MSB-first across 4 bytes):
 *
 *   Byte 0: [dc:3=0][R/W:1][CMD:4]
 *
 *   DAC80004 (16-bit data, all bits used):
 *     Byte 1: [ADDR:4][DAT15:12]
 *     Byte 2: [DAT11:4]
 *     Byte 3: [DAT3:0][MOD:4]
 *
 *   DAC70004 (14-bit data, right-justified in dat field):
 *     Byte 1: [ADDR:4][DAT13:10]
 *     Byte 2: [DAT9:2]
 *     Byte 3: [DAT1:0][00:2][MOD:4]
 *
 *   DAC60004 (12-bit data, right-justified in dat field):
 *     Byte 1: [ADDR:4][DAT11:8]
 *     Byte 2: [DAT7:0]
 *     Byte 3: [00:4][MOD:4]
 */

#include <gtest/gtest.h>
#include "mock_if.h"

class BitPackingTest : public ::testing::Test {
protected:
    dacx0004_dev_t dev80{}, dev70{}, dev60{};

    void SetUp() override {
        g_mock.reset();
        dacx0004_init_dev(&dev80, DAC80004, &mock_if, nullptr);
        dacx0004_init_dev(&dev70, DAC70004, &mock_if, nullptr);
        dacx0004_init_dev(&dev60, DAC60004, &mock_if, nullptr);
        g_mock.reset();
    }

    // Call write_sr and return the 4 bytes that were shifted out.
    void pack(dacx0004_dev_t* dev, dacx0004_sr_t sr, uint8_t out[4]) {
        g_mock.reset();
        ASSERT_EQ(dacx0004_write_sr(dev, sr), DACX0004_STAT_OK);
        ASSERT_EQ(g_mock.calls.size(), 1u);
        ASSERT_EQ(g_mock.calls[0].fn, MockFn::SHIFT_SR);
        memcpy(out, g_mock.calls[0].data, 4);
    }
};

// ── Byte 0 (R/W + CMD) — same for all variants ───────────────────────────────

TEST_F(BitPackingTest, Byte0_WriteCmd) {
    // Rw=WRITE(0), cmd=WRITEn_UPDATEn(3) → byte0 = (0<<4)|3 = 0x03
    dacx0004_sr_t sr{}; sr.Rw = DACX0004_RW_WRITE; sr.cmd = DACX0004_CMD_WRITEn_UPDATEn;
    uint8_t b[4];
    pack(&dev80, sr, b);
    EXPECT_EQ(b[0], 0x03u);
}

TEST_F(BitPackingTest, Byte0_ReadCmd) {
    // Rw=READ(1), cmd=STATUS_REG(0xD) → byte0 = (1<<4)|0xD = 0x1D
    dacx0004_sr_t sr{}; sr.Rw = DACX0004_RW_READ; sr.cmd = DACX0004_CMD_STATUS_REG;
    uint8_t b[4];
    pack(&dev80, sr, b);
    EXPECT_EQ(b[0], 0x1Du);
}

// ── DAC80004 ──────────────────────────────────────────────────────────────────

TEST_F(BitPackingTest, DAC80004_AllZeros) {
    dacx0004_sr_t sr{};
    uint8_t b[4];
    pack(&dev80, sr, b);
    EXPECT_EQ(b[0], 0x00u);
    EXPECT_EQ(b[1], 0x00u);
    EXPECT_EQ(b[2], 0x00u);
    EXPECT_EQ(b[3], 0x00u);
}

TEST_F(BitPackingTest, DAC80004_KnownValue) {
    // cmd=3, add=0, dat=0xABCD, mod=5
    // b0 = 0x03
    // b1 = (0<<4)|(0xABCD>>12) = 0x0A
    // b2 = (0xABCD>>4)&0xFF    = 0xBC
    // b3 = ((0xABCD&0xF)<<4)|5 = 0xD5
    dacx0004_sr_t sr{};
    sr.Rw = DACX0004_RW_WRITE; sr.cmd = DACX0004_CMD_WRITEn_UPDATEn;
    sr.add = DACX0004_ADD_A; sr.dat = 0xABCD; sr.mod = 0x5;
    uint8_t b[4];
    pack(&dev80, sr, b);
    EXPECT_EQ(b[0], 0x03u);
    EXPECT_EQ(b[1], 0x0Au);
    EXPECT_EQ(b[2], 0xBCu);
    EXPECT_EQ(b[3], 0xD5u);
}

TEST_F(BitPackingTest, DAC80004_AllOnes) {
    // Rw=1, cmd=0xF, add=0xF, dat=0xFFFF, mod=0xF
    // b0=0x1F, b1=0xFF, b2=0xFF, b3=0xFF
    dacx0004_sr_t sr{};
    sr.Rw = DACX0004_RW_READ; sr.cmd = 0xF;
    sr.add = DACX0004_ADD_ALL; sr.dat = 0xFFFF; sr.mod = 0xF;
    uint8_t b[4];
    pack(&dev80, sr, b);
    EXPECT_EQ(b[0], 0x1Fu);
    EXPECT_EQ(b[1], 0xFFu);
    EXPECT_EQ(b[2], 0xFFu);
    EXPECT_EQ(b[3], 0xFFu);
}

TEST_F(BitPackingTest, DAC80004_Alternating_0xAAAA) {
    // cmd=0x5, add=0xA, dat=0xAAAA, mod=0x5
    // b0 = (0<<4)|5 = 0x05
    // b1 = (0xA<<4)|(0xAAAA>>12) = 0xAA
    // b2 = (0xAAAA>>4)&0xFF      = 0xAA
    // b3 = ((0xA<<4)|0x5)        = 0xA5
    dacx0004_sr_t sr{};
    sr.Rw = DACX0004_RW_WRITE; sr.cmd = 0x5;
    sr.add = 0xA; sr.dat = 0xAAAA; sr.mod = 0x5;
    uint8_t b[4];
    pack(&dev80, sr, b);
    EXPECT_EQ(b[0], 0x05u);
    EXPECT_EQ(b[1], 0xAAu);
    EXPECT_EQ(b[2], 0xAAu);
    EXPECT_EQ(b[3], 0xA5u);
}

TEST_F(BitPackingTest, DAC80004_AddrIsolated) {
    // add=0x5, all other fields 0 → b1 = (0x5<<4)|0 = 0x50
    dacx0004_sr_t sr{}; sr.add = 0x5;
    uint8_t b[4];
    pack(&dev80, sr, b);
    EXPECT_EQ(b[1], 0x50u);
    EXPECT_EQ(b[2], 0x00u);
    EXPECT_EQ(b[3], 0x00u);
}

// ── DAC70004 ──────────────────────────────────────────────────────────────────

TEST_F(BitPackingTest, DAC70004_AllZeros) {
    dacx0004_sr_t sr{};
    uint8_t b[4];
    pack(&dev70, sr, b);
    EXPECT_EQ(b[0], 0x00u);
    EXPECT_EQ(b[1], 0x00u);
    EXPECT_EQ(b[2], 0x00u);
    EXPECT_EQ(b[3], 0x00u);
}

TEST_F(BitPackingTest, DAC70004_MaxData) {
    // dat=0x3FFF (all 14 bits set), mod=0, add=0
    // b1 = 0|(0x3FFF&0x3C00)>>10 = 0x0F
    // b2 = (0x3FFF&0x03FC)>>2    = 0xFF
    // b3 = ((0x3FFF&3)<<6)|0     = 0xC0
    dacx0004_sr_t sr{};
    sr.Rw = DACX0004_RW_WRITE; sr.cmd = DACX0004_CMD_WRITEn_UPDATEn;
    sr.dat = 0x3FFF;
    uint8_t b[4];
    pack(&dev70, sr, b);
    EXPECT_EQ(b[0], 0x03u);
    EXPECT_EQ(b[1], 0x0Fu);
    EXPECT_EQ(b[2], 0xFFu);
    EXPECT_EQ(b[3], 0xC0u);
}

TEST_F(BitPackingTest, DAC70004_Alternating) {
    // dat=0x2AAA, mod=5, add=0
    // 0x2AAA = 0010 1010 1010 1010
    // b1 = 0|(0x2AAA&0x3C00)>>10 = 0|0xA = 0x0A
    // b2 = (0x2AAA&0x03FC)>>2    = 0x02A8>>2 = 0xAA
    // b3 = ((0x2AAA&3)<<6)|5     = (2<<6)|5  = 0x85
    dacx0004_sr_t sr{};
    sr.Rw = DACX0004_RW_WRITE; sr.cmd = DACX0004_CMD_WRITEn_UPDATEn;
    sr.dat = 0x2AAA; sr.mod = 0x5;
    uint8_t b[4];
    pack(&dev70, sr, b);
    EXPECT_EQ(b[0], 0x03u);
    EXPECT_EQ(b[1], 0x0Au);
    EXPECT_EQ(b[2], 0xAAu);
    EXPECT_EQ(b[3], 0x85u);
}

TEST_F(BitPackingTest, DAC70004_ModIsolated) {
    // dat=0, mod=0xA → b3 = 0|0xA = 0x0A
    dacx0004_sr_t sr{}; sr.mod = 0xA;
    uint8_t b[4];
    pack(&dev70, sr, b);
    EXPECT_EQ(b[1], 0x00u);
    EXPECT_EQ(b[2], 0x00u);
    EXPECT_EQ(b[3], 0x0Au);
}

// ── DAC60004 ──────────────────────────────────────────────────────────────────

TEST_F(BitPackingTest, DAC60004_AllZeros) {
    dacx0004_sr_t sr{};
    uint8_t b[4];
    pack(&dev60, sr, b);
    EXPECT_EQ(b[0], 0x00u);
    EXPECT_EQ(b[1], 0x00u);
    EXPECT_EQ(b[2], 0x00u);
    EXPECT_EQ(b[3], 0x00u);
}

TEST_F(BitPackingTest, DAC60004_MaxData) {
    // dat=0x0FFF (all 12 bits set), mod=0, add=0
    // b1 = 0|(0x0FFF&0x0F00)>>8 = 0x0F
    // b2 = 0x0FFF&0x00FF        = 0xFF
    // b3 = 0 (no data bits)
    dacx0004_sr_t sr{};
    sr.Rw = DACX0004_RW_WRITE; sr.cmd = DACX0004_CMD_WRITEn_UPDATEn;
    sr.dat = 0x0FFF;
    uint8_t b[4];
    pack(&dev60, sr, b);
    EXPECT_EQ(b[0], 0x03u);
    EXPECT_EQ(b[1], 0x0Fu);
    EXPECT_EQ(b[2], 0xFFu);
    EXPECT_EQ(b[3], 0x00u);
}

TEST_F(BitPackingTest, DAC60004_Alternating) {
    // dat=0x0A5A, mod=0xF, add=0
    // b1 = 0|(0x0A5A&0x0F00)>>8 = 0x0A
    // b2 = 0x0A5A&0xFF          = 0x5A
    // b3 = 0xF (mod only)
    dacx0004_sr_t sr{};
    sr.Rw = DACX0004_RW_WRITE; sr.cmd = DACX0004_CMD_WRITEn_UPDATEn;
    sr.dat = 0x0A5A; sr.mod = 0xF;
    uint8_t b[4];
    pack(&dev60, sr, b);
    EXPECT_EQ(b[0], 0x03u);
    EXPECT_EQ(b[1], 0x0Au);
    EXPECT_EQ(b[2], 0x5Au);
    EXPECT_EQ(b[3], 0x0Fu);
}

TEST_F(BitPackingTest, DAC60004_ModIsolated) {
    // dat=0, mod=0xA → b2=0, b3=0x0A
    dacx0004_sr_t sr{}; sr.mod = 0xA;
    uint8_t b[4];
    pack(&dev60, sr, b);
    EXPECT_EQ(b[1], 0x00u);
    EXPECT_EQ(b[2], 0x00u);
    EXPECT_EQ(b[3], 0x0Au);
}

// ── Address encoding is shared across all variants ────────────────────────────

TEST_F(BitPackingTest, AllVariants_AddressInUpperNibbleOfByte1) {
    dacx0004_sr_t sr{}; sr.add = 0xB;
    uint8_t b80[4], b70[4], b60[4];
    pack(&dev80, sr, b80);
    pack(&dev70, sr, b70);
    pack(&dev60, sr, b60);
    EXPECT_EQ((b80[1] >> 4) & 0xFu, 0xBu);
    EXPECT_EQ((b70[1] >> 4) & 0xFu, 0xBu);
    EXPECT_EQ((b60[1] >> 4) & 0xFu, 0xBu);
}
