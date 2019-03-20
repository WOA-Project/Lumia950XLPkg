/* Copyright (c) 2011, The Linux Foundation. All rights reserved.

* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of The Linux Foundation nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef __GSBI_H_
#define __GSBI_H_

/* GSBI Registers */
#define GSBI_CTRL_REG(base) ((base) + 0x0)

#define GSBI_CTRL_REG_PROTOCOL_CODE_S 4
#define GSBI_PROTOCOL_CODE_I2C 0x2
#define GSBI_PROTOCOL_CODE_SPI 0x3
#define GSBI_PROTOCOL_CODE_UART_FLOW 0x4
#define GSBI_PROTOCOL_CODE_I2C_UART 0x6

#define GSBI_HCLK_CTL_S 4
#define GSBI_HCLK_CTL_CLK_ENA 0x1

enum {
  GSBI_ID_1 = 1,
  GSBI_ID_2,
  GSBI_ID_3,
  GSBI_ID_4,
  GSBI_ID_5,
  GSBI_ID_6,
  GSBI_ID_7,
  GSBI_ID_8,
  GSBI_ID_9,
  GSBI_ID_10,
  GSBI_ID_11,
  GSBI_ID_12,
  GSBI_ID_MAX,
};

#endif
