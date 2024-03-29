/*
Copyright 2019 Owen Lyke

Permission is hereby granted, free of charge, to any person obtaining a copy of this 
software and associated documentation files (the "Software"), to deal in the Software 
without restriction, including without limitation the rights to use, copy, modify, 
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be 
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _DACX0004_PLATFORM_ESP32_H_
#define _DACX0004_PLATFORM_ESP32_H_

#include "dacx0004.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

typedef struct _dax_if_esp32_arg_t {
  spi_device_handle_t     spi;
  spi_host_device_t       host;
  uint32_t                spi_q_size;
  uint32_t                clk_freq;
  int                     clk_pin;
  int                     mosi_pin;
  int                     miso_pin;
  int                     sync_pin;
  int                     ldac_pin;
  int                     clr_pin;
} dax_if_esp32_arg_t;

extern dacx0004_if_t dax_if_esp32;

#endif // _DACX0004_PLATFORM_ESP32_H_

// This header file provides an ESP32 implementation of the DAX interface
// Example Usage:
/*

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dacx0004_platform_esp32.h"

#define PIN_NUM_MISO -1
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_LDAC 27
#define PIN_NUM_SYNC 26
#define PIN_NUM_CLR 25

#define HOST            VSPI_HOST
#define CLOCK_FREQ      40000000    // 50 MHz max for DAC80004
#define MAX_XFER_SIZE   0           // defaults to 4096 if set to 0
#define MAX_Q_SIZE      1
#define DMA_CHANNEL     1

// Globals
dacx0004_dev_t         dax = {0};
dax_if_esp32_arg_t  if_args = {
    .spi = NULL,
    .host = HOST,
    .spi_q_size = MAX_Q_SIZE,
    .sync_pin = PIN_NUM_SYNC,
    .ldac_pin = PIN_NUM_LDAC,
    .clr_pin = PIN_NUM_CLR,
    .mosi_pin = PIN_NUM_MOSI,
    .miso_pin = PIN_NUM_MISO,
    .clk_pin = PIN_NUM_CLK,
    .clk_freq = CLOCK_FREQ,
};
da80004_sr_t sr = {
    .Rw = DACX0004_RW_WRITE,
    .cmd = DACX0004_CMD_WRITEn_UPDATEn,
    .add = DACX0004_ADD_A,
    .dat = 0xDEAD,
    // .mod = 0,
};

void app_main(void)
{
  dacx0004_status_e dax_ret = DACX0004_STAT_OK;

  // SPI bus init may depend on other devices so it is handled separately
  spi_bus_config_t buscfg={
        .miso_io_num        = if_args.miso_pin,
        .mosi_io_num        = if_args.mosi_pin,
        .sclk_io_num        = if_args.clk_pin,
        .quadwp_io_num      = -1,               // leave these as -1 for high speed ( > 26 MHz ) capability
        .quadhd_io_num      = -1,               // leave these as -1 for high speed ( > 26 MHz ) capability
        .max_transfer_sz    = MAX_XFER_SIZE,    // Must be at least 4, or 0 for default of 4096
  };
  ESP_ERROR_CHECK(spi_bus_initialize(if_args.host, &buscfg, DMA_CHANNEL));

  // DAX operations
  dax_ret = dacx0004_init_dev(&dax, DAC80004, &dax_if_esp32, &if_args);
  dax_ret = dacx0004_write_sr(&dax, sr);     // Writes the command to the DAX

  while(1){
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

*/
