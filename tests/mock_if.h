#pragma once

#include <vector>
#include <cstdint>
#include <cstring>

extern "C" {
#include "dacx0004.h"
}

enum class MockFn { SHIFT_SR, SHIFT_SR_RW, SET_LDAC, SET_CLR, SET_SYNC };

struct MockCall {
    MockFn   fn;
    uint8_t  data[4];   // captured tx bytes (shift calls)
    uint32_t len;       // byte count (shift calls)
    bool     level;     // driven level (gpio calls)
};

struct MockState {
    std::vector<MockCall> calls;
    uint8_t               rx_response[4];  // bytes returned by shift_sr_rw
    dacx0004_status_e     next_status;

    void reset() {
        calls.clear();
        memset(rx_response, 0, sizeof(rx_response));
        next_status = DACX0004_STAT_OK;
    }
};

extern MockState      g_mock;
extern dacx0004_if_t  mock_if;         // all callbacks present
extern dacx0004_if_t  mock_if_no_rw;   // shift_sr_rw absent
extern dacx0004_if_t  mock_if_no_gpio; // set_ldac and set_clr absent
