#include "mock_if.h"

MockState g_mock;

static dacx0004_status_e mock_shift_sr(uint8_t* pdat, uint32_t len, void* /*arg*/) {
    MockCall c{};
    c.fn  = MockFn::SHIFT_SR;
    c.len = len;
    memcpy(c.data, pdat, len < 4u ? len : 4u);
    g_mock.calls.push_back(c);
    return g_mock.next_status;
}

static dacx0004_status_e mock_shift_sr_rw(uint8_t* tx, uint8_t* rx, uint32_t len, void* /*arg*/) {
    MockCall c{};
    c.fn  = MockFn::SHIFT_SR_RW;
    c.len = len;
    memcpy(c.data, tx, len < 4u ? len : 4u);
    g_mock.calls.push_back(c);
    if (rx && len >= 4u) memcpy(rx, g_mock.rx_response, 4);
    return g_mock.next_status;
}

static dacx0004_status_e mock_set_ldac(bool lvl, void* /*arg*/) {
    MockCall c{};
    c.fn    = MockFn::SET_LDAC;
    c.level = lvl;
    g_mock.calls.push_back(c);
    return g_mock.next_status;
}

static dacx0004_status_e mock_set_clr(bool lvl, void* /*arg*/) {
    MockCall c{};
    c.fn    = MockFn::SET_CLR;
    c.level = lvl;
    g_mock.calls.push_back(c);
    return g_mock.next_status;
}

dacx0004_if_t mock_if = {
    .shift_sr    = mock_shift_sr,
    .shift_sr_rw = mock_shift_sr_rw,
    .set_sync    = nullptr,
    .set_ldac    = mock_set_ldac,
    .set_clr     = mock_set_clr,
};

dacx0004_if_t mock_if_no_rw = {
    .shift_sr    = mock_shift_sr,
    .shift_sr_rw = nullptr,
    .set_sync    = nullptr,
    .set_ldac    = mock_set_ldac,
    .set_clr     = mock_set_clr,
};

dacx0004_if_t mock_if_no_gpio = {
    .shift_sr    = mock_shift_sr,
    .shift_sr_rw = mock_shift_sr_rw,
    .set_sync    = nullptr,
    .set_ldac    = nullptr,
    .set_clr     = nullptr,
};
