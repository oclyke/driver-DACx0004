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

#include "dacx0004_platform_esp32.h"

// ESP32-specific interface functions for the DACX0004
dacx0004_status_e dacx0004_esp32_shift_sr(uint8_t* pdat, uint32_t len, void* arg);
dacx0004_status_e dacx0004_esp32_set_sync(bool lvl, void* arg);
dacx0004_status_e dacx0004_esp32_set_ldac(bool lvl, void* arg);
dacx0004_status_e dacx0004_esp32_set_clr(bool lvl, void* arg);

dacx0004_if_t dax_if_esp32 = {
  .shift_sr = dacx0004_esp32_shift_sr,
  .set_sync = NULL,                     // sync is handled by SPI master
  .set_ldac = dacx0004_esp32_set_ldac,
  .set_clr = dacx0004_esp32_set_clr,
};

dacx0004_status_e dacx0004_esp32_shift_sr(uint8_t* pdat, uint32_t len, void* arg){
  dax_if_esp32_arg_t* if_args = (dax_if_esp32_arg_t*)arg;
  const uint8_t xfer_size = 4;

  if(len % xfer_size != 0){ return DACX0004_STAT_ERR_INVALID_ARG; }

  if(if_args->spi == NULL){
    spi_device_interface_config_t devcfg={
        .clock_speed_hz = if_args->clk_freq,
        .mode = 2,
        .spics_io_num = if_args->sync_pin,
        .queue_size = if_args->spi_q_size,
    };
    spi_bus_add_device(if_args->host, &devcfg, &(if_args->spi));
  }

  esp_err_t ret = ESP_OK;
  for(uint32_t ux = 0; ux < (len / xfer_size); ux++){
    spi_transaction_t trans = {
      .length    = xfer_size * 8,
      .tx_buffer = (void*)(pdat + (ux * xfer_size)),
    };
    ret |= spi_device_polling_transmit(if_args->spi, &trans);
  }

  return (ret == ESP_OK) ? DACX0004_STAT_OK : DACX0004_STAT_ERR;
}

dacx0004_status_e dacx0004_esp32_set_sync(bool lvl, void* arg){
  // if(arg == NULL){ return DACX0004_STAT_ERR; }
  dax_if_esp32_arg_t* if_args = (dax_if_esp32_arg_t*)arg;
  static bool sync_initialized = false;
  if(!sync_initialized){
    gpio_pad_select_gpio(if_args->sync_pin);
    gpio_set_direction(if_args->sync_pin, GPIO_MODE_OUTPUT);
    sync_initialized = true;
  }
  gpio_set_level(if_args->sync_pin, lvl);
  return DACX0004_STAT_OK;
}

dacx0004_status_e dacx0004_esp32_set_ldac(bool lvl, void* arg){
  // if(arg == NULL){ return DACX0004_STAT_ERR; }
  dax_if_esp32_arg_t* if_args = (dax_if_esp32_arg_t*)arg;
  static bool ldac_initialized = false;
  if(!ldac_initialized){
    gpio_pad_select_gpio(if_args->ldac_pin);
    gpio_set_direction(if_args->ldac_pin, GPIO_MODE_OUTPUT);
    ldac_initialized = true;
  }
  gpio_set_level(if_args->ldac_pin, lvl);
  return DACX0004_STAT_OK;
}

dacx0004_status_e dacx0004_esp32_set_clr(bool lvl, void* arg){
  // if(arg == NULL){ return DACX0004_STAT_ERR; }
  dax_if_esp32_arg_t* if_args = (dax_if_esp32_arg_t*)arg;
  static bool clr_initialized = false;
  if(!clr_initialized){
    gpio_pad_select_gpio(if_args->clr_pin);
    gpio_set_direction(if_args->clr_pin, GPIO_MODE_OUTPUT);
    clr_initialized = true;
  }
  gpio_set_level(if_args->clr_pin, lvl);
  return DACX0004_STAT_OK;
}
