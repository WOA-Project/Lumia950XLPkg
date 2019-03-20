/* Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Fundation, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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
 *
 */

#include <Library/LKEnvLib.h>

#include <Library/MallocLib.h>
#include <Protocol/QcomRpm.h>

#include "rpm-ipc.h"
// Must come in order
#include "rpm-smd.h"

void fill_kvp_object(kvp_data **kdata, uint32_t *data, uint32_t len)
{
  *kdata = (kvp_data *)memalign(CACHE_LINE, ROUNDUP(len, CACHE_LINE));
  ASSERT(*kdata);

  memcpy(*kdata, data + 2, len);
}

void free_kvp_object(kvp_data **kdata)
{
  if (*kdata)
    free(*kdata);
}

int rpm_send_data(uint32_t *data, uint32_t len, msg_type type)
{
  return rpm_smd_send_data(data, len, type);
}

void rpm_clk_enable(uint32_t *data, uint32_t len)
{
  if (rpm_send_data(data, len, RPM_REQUEST_TYPE)) {
    dprintf(CRITICAL, "Clock enable failure\n");
    ASSERT(0);
  }
}
