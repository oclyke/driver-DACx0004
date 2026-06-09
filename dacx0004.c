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

#include "dacx0004.h"

#define DACX0004_DAT0(sr) ((uint8_t)( ((sr.Rw & 0x01) << 4) | ((sr.cmd & 0x0F) << 0) ))

#define DAC80004_DAT1(sr) ((uint8_t)( ((sr.add & 0x0F) << 4) | (((uint16_t)sr.dat & 0xF000) >> 12) ))
#define DAC70004_DAT1(sr) ((uint8_t)( ((sr.add & 0x0F) << 4) | (((uint16_t)sr.dat & 0x3C00) >> 10) ))
#define DAC60004_DAT1(sr) ((uint8_t)( ((sr.add & 0x0F) << 4) | (((uint16_t)sr.dat & 0x0F00) >> 8) ))

#define DAC80004_DAT2(sr) ((uint8_t)( (((uint16_t)sr.dat & 0x0FF0) >> 4) ))
#define DAC70004_DAT2(sr) ((uint8_t)( (((uint16_t)sr.dat & 0x03FC) >> 2) ))
#define DAC60004_DAT2(sr) ((uint8_t)( (((uint16_t)sr.dat & 0x00FF) >> 0) ))

#define DAC80004_DAT3(sr) ((uint8_t)( (((uint16_t)sr.dat & 0x000F) << 4) | ((sr.mod & 0x0F) << 0) ))
#define DAC70004_DAT3(sr) ((uint8_t)( (((uint16_t)sr.dat & 0x0003) << 6) | ((sr.mod & 0x0F) << 0) ))
#define DAC60004_DAT3(sr) ((uint8_t)( (        ( 0x00 << 0)              | ((sr.mod & 0x0F) << 0) )))



dacx0004_status_e dacx0004_init_dev(dacx0004_dev_t* pdev, dacx0004_ver_e ver, dacx0004_if_t* pif, void* arg){
  if(pdev == NULL)          { return DACX0004_STAT_ERR_INVALID_ARG; }
  if(pif == NULL)           { return DACX0004_STAT_ERR_INVALID_ARG; }
  if(ver >= DACX0004_VER_NUM){ return DACX0004_STAT_ERR_UNKNOWN_VER; }

  pdev->_if = pif;
  pdev->_ver = ver;
  pdev->_arg = arg;

  // Set up control pins
  if(pdev->_if->set_clr  != NULL){ pdev->_if->set_clr(true, pdev->_arg); }
  if(pdev->_if->set_ldac != NULL){ pdev->_if->set_ldac(false, pdev->_arg); }
  if(pdev->_if->set_sync != NULL){ pdev->_if->set_sync(true, pdev->_arg); }

  return DACX0004_STAT_OK;
}


dacx0004_status_e dacx0004_write_sr(dacx0004_dev_t* pdev, dacx0004_sr_t sr){
  if(pdev == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }
  if(pdev->_if == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }
  if(pdev->_if->shift_sr == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }

  const uint8_t len = 4;
  uint8_t dat[len];

  dat[0] = DACX0004_DAT0(sr);
  switch(pdev->_ver){
    case DAC80004 :
      dat[1] = DAC80004_DAT1(sr);
      dat[2] = DAC80004_DAT2(sr);
      dat[3] = DAC80004_DAT3(sr);
      break;
    case DAC70004 :
      dat[1] = DAC70004_DAT1(sr);
      dat[2] = DAC70004_DAT2(sr);
      dat[3] = DAC70004_DAT3(sr);
      break;
    case DAC60004 :
      dat[1] = DAC60004_DAT1(sr);
      dat[2] = DAC60004_DAT2(sr);
      dat[3] = DAC60004_DAT3(sr);
      break;
    default :
      return DACX0004_STAT_ERR_UNKNOWN_VER;
      break;
  }

  return pdev->_if->shift_sr(dat, len, pdev->_arg);
}

dacx0004_status_e dacx0004_write_channel(dacx0004_dev_t* pdev, dacx0004_add_e channel, uint16_t value){
  dacx0004_sr_t sr = { .Rw = DACX0004_RW_WRITE, .cmd = DACX0004_CMD_WRITEn, .add = channel, .dat = value, .mod = 0 };
  return dacx0004_write_sr(pdev, sr);
}

dacx0004_status_e dacx0004_update_channel(dacx0004_dev_t* pdev, dacx0004_add_e channel){
  dacx0004_sr_t sr = { .Rw = DACX0004_RW_WRITE, .cmd = DACX0004_CMD_UPDATEn, .add = channel, .dat = 0, .mod = 0 };
  return dacx0004_write_sr(pdev, sr);
}

dacx0004_status_e dacx0004_write_update_channel(dacx0004_dev_t* pdev, dacx0004_add_e channel, uint16_t value){
  dacx0004_sr_t sr = { .Rw = DACX0004_RW_WRITE, .cmd = DACX0004_CMD_WRITEn_UPDATEn, .add = channel, .dat = value, .mod = 0 };
  return dacx0004_write_sr(pdev, sr);
}

dacx0004_status_e dacx0004_read_sr(dacx0004_dev_t* pdev, dacx0004_sr_t sr, dacx0004_sr_t* result){
  if(pdev == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }
  if(pdev->_if == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }
  if(pdev->_if->shift_sr_rw == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }
  if(result == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }

  const uint8_t len = 4;
  uint8_t tx[len];
  uint8_t rx[len];

  sr.Rw = DACX0004_RW_READ;
  tx[0] = DACX0004_DAT0(sr);
  switch(pdev->_ver){
    case DAC80004:
      tx[1] = DAC80004_DAT1(sr);
      tx[2] = DAC80004_DAT2(sr);
      tx[3] = DAC80004_DAT3(sr);
      break;
    case DAC70004:
      tx[1] = DAC70004_DAT1(sr);
      tx[2] = DAC70004_DAT2(sr);
      tx[3] = DAC70004_DAT3(sr);
      break;
    case DAC60004:
      tx[1] = DAC60004_DAT1(sr);
      tx[2] = DAC60004_DAT2(sr);
      tx[3] = DAC60004_DAT3(sr);
      break;
    default:
      return DACX0004_STAT_ERR_UNKNOWN_VER;
  }

  dacx0004_status_e ret = pdev->_if->shift_sr_rw(tx, rx, len, pdev->_arg);
  if(ret != DACX0004_STAT_OK){ return ret; }

  result->_dc = 0;
  result->Rw  = (rx[0] >> 4) & 0x01;
  result->cmd = (rx[0] >> 0) & 0x0F;
  result->add = (rx[1] >> 4) & 0x0F;
  result->mod = (rx[3] >> 0) & 0x0F;
  switch(pdev->_ver){
    case DAC80004:
      result->dat = ((uint16_t)(rx[1] & 0x0F) << 12) |
                    ((uint16_t) rx[2]          <<  4) |
                    ((uint16_t)(rx[3] >>    4) &  0x0F);
      break;
    case DAC70004:
      result->dat = ((uint16_t)(rx[1] & 0x0F) << 10) |
                    ((uint16_t) rx[2]          <<  2) |
                    ((uint16_t)(rx[3] >>    6) &  0x03);
      break;
    case DAC60004:
      result->dat = ((uint16_t)(rx[1] & 0x0F) <<  8) |
                    ((uint16_t) rx[2]);
      break;
    default:
      return DACX0004_STAT_ERR_UNKNOWN_VER;
  }

  return DACX0004_STAT_OK;
}

dacx0004_status_e dacx0004_software_reset(dacx0004_dev_t* pdev){
  dacx0004_sr_t sr = { .Rw = DACX0004_RW_WRITE, .cmd = DACX0004_CMD_SOFTWARE_RESET, .add = 0, .dat = 0, .mod = 0 };
  return dacx0004_write_sr(pdev, sr);
}

dacx0004_status_e dacx0004_software_clear(dacx0004_dev_t* pdev){
  dacx0004_sr_t sr = { .Rw = DACX0004_RW_WRITE, .cmd = DACX0004_CMD_SOFTWARE_CLEAR, .add = 0, .dat = 0, .mod = 0 };
  return dacx0004_write_sr(pdev, sr);
}

dacx0004_status_e dacx0004_set_power(dacx0004_dev_t* pdev, dacx0004_add_e channel, dacx0004_pwr_e mode){
  dacx0004_sr_t sr = { .Rw = DACX0004_RW_WRITE, .cmd = DACX0004_CMD_PWRnc, .add = channel, .dat = 0, .mod = (uint8_t)mode };
  return dacx0004_write_sr(pdev, sr);
}

dacx0004_status_e dacx0004_set_clear_mode(dacx0004_dev_t* pdev, dacx0004_clm_e mode){
  dacx0004_sr_t sr = { .Rw = DACX0004_RW_WRITE, .cmd = DACX0004_CMD_CLEAR_MODE_REG, .add = 0, .dat = 0, .mod = (uint8_t)mode };
  return dacx0004_write_sr(pdev, sr);
}

dacx0004_status_e dacx0004_ldac_pulse(dacx0004_dev_t* pdev){
  if(pdev == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }
  if(pdev->_if == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }
  if(pdev->_if->set_ldac == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }
  dacx0004_status_e ret = pdev->_if->set_ldac(true, pdev->_arg);
  if(ret != DACX0004_STAT_OK){ return ret; }
  return pdev->_if->set_ldac(false, pdev->_arg);
}

dacx0004_status_e dacx0004_clr_pulse(dacx0004_dev_t* pdev){
  if(pdev == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }
  if(pdev->_if == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }
  if(pdev->_if->set_clr == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }
  dacx0004_status_e ret = pdev->_if->set_clr(false, pdev->_arg);
  if(ret != DACX0004_STAT_OK){ return ret; }
  return pdev->_if->set_clr(true, pdev->_arg);
}

dacx0004_status_e dacx0004_format_sr(dacx0004_dev_t* pdev, dacx0004_sr_t sr, uint8_t* dest, uint32_t len){
  if(pdev == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }
  if(dest == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }

  uint8_t d0 = DACX0004_DAT0(sr);
  uint8_t d1, d2, d3;
  uint32_t full_packs = 0;
  uint8_t remainder = len%4;

  switch(pdev->_ver){
    case DAC80004 :
      d1 = DAC80004_DAT1(sr);
      d2 = DAC80004_DAT2(sr);
      d3 = DAC80004_DAT3(sr);
      break;
    case DAC70004 :
      d1 = DAC70004_DAT1(sr);
      d2 = DAC70004_DAT2(sr);
      d3 = DAC70004_DAT3(sr);
      break;
    case DAC60004 :
      d1 = DAC60004_DAT1(sr);
      d2 = DAC60004_DAT2(sr);
      d3 = DAC60004_DAT3(sr);
      break;
    default :
      return DACX0004_STAT_ERR_UNKNOWN_VER;
      break;
  }

  for(full_packs = 0; full_packs < (len/4); full_packs++){
    *(dest + (4*full_packs) + 0) = d0;
    *(dest + (4*full_packs) + 1) = d1;
    *(dest + (4*full_packs) + 2) = d2;
    *(dest + (4*full_packs) + 3) = d3;
  }

  switch(remainder){  // fall-through intended
    case 3 :
      *(dest + (4*full_packs) + 2) = d2;
    case 2 :
      *(dest + (4*full_packs) + 1) = d1;
    case 1 :
      *(dest + (4*full_packs) + 0) = d0;
    default :
      break;
  }

  return DACX0004_STAT_OK;
}
