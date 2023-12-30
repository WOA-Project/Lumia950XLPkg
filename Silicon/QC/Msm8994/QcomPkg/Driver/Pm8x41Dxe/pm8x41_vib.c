/* Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation, Inc. nor the names of its
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

#include <Library/LKEnvLib.h>

#include <Device/pm8x41.h>

#include <Library/QcomPm8x41Lib.h>

#include "pm8x41_p.h"
// Must come in order
#include "pm_vib.h"

#define QPNP_VIB_EN BIT(7)

/* Turn on vibrator */
void pm_vib_turn_on(void)
{
  uint8_t val;

  val = gPm8x41->pm8x41_reg_read(QPNP_VIB_VTG_CTL);
  val &= ~QPNP_VIB_VTG_SET_MASK;
  val |= (QPNP_VIB_DEFAULT_VTG_LVL & QPNP_VIB_VTG_SET_MASK);
  gPm8x41->pm8x41_reg_write(QPNP_VIB_VTG_CTL, val);

  val = gPm8x41->pm8x41_reg_read(QPNP_VIB_EN_CTL);
  val |= QPNP_VIB_EN;
  gPm8x41->pm8x41_reg_write(QPNP_VIB_EN_CTL, val);
}

/* Turn off vibrator */
void pm_vib_turn_off(void)
{
  uint8_t val;

  val = gPm8x41->pm8x41_reg_read(QPNP_VIB_EN_CTL);
  val &= ~QPNP_VIB_EN;
  gPm8x41->pm8x41_reg_write(QPNP_VIB_EN_CTL, val);
}
