/* Copyright (c) 2014, The Linux Foundation. All rights reserved.
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

#include <PiDxe.h>

#include <Library/LKEnvLib.h>

#include <Library/EfiResetSystemLib.h>
#include <Library/InterruptsLib.h>
#include <Library/MallocLib.h>
#include <Library/QcomSmemLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/HardwareInterrupt.h>

#include "smd.h"

#define SMD_CHANNEL_ACCESS_RETRY 1000000

STATIC smd_channel_alloc_entry_t *smd_channel_alloc_entry;
STATIC EFI_EVENT smd_closed            = (EFI_EVENT)NULL;
STATIC BOOLEAN smd_exit_bs_in_progress = FALSE;
STATIC BOOLEAN smd_channel_alloc_freed = FALSE;

static void smd_write_state(smd_channel_info_t *ch, uint32_t state)
{
  if (state == SMD_SS_OPENED) {
    ch->port_info->ch0.DTR_DSR = 1;
    ch->port_info->ch0.CTS_RTS = 1;
    ch->port_info->ch0.CD      = 1;
  }
  else {
    ch->port_info->ch0.DTR_DSR = 0;
    ch->port_info->ch0.CTS_RTS = 0;
    ch->port_info->ch0.CD      = 0;
  }

  ch->port_info->ch0.stream_state = state;
}

static void smd_state_update(smd_channel_info_t *ch, uint32_t flag)
{
  ch->port_info->ch0.state_updated = flag;
}

int smd_get_channel_entry(smd_channel_info_t *ch, uint32_t ch_type)
{
  int i = 0;

  for (i = 0; i < SMEM_NUM_SMD_STREAM_CHANNELS; i++) {
    if ((smd_channel_alloc_entry[i].ctype & 0xFF) == ch_type) {
      memcpy(
          &ch->alloc_entry, &smd_channel_alloc_entry[i],
          sizeof(smd_channel_alloc_entry_t));
      break;
    }
  }

  /* Channel not found, retry again */
  if (i == SMEM_NUM_SMD_STREAM_CHANNELS) {
    dprintf(SPEW, "Channel not found, wait and retry for the update\n");
    return -1;
  }

  return 0;
}

int smd_get_channel_info(smd_channel_info_t *ch, uint32_t ch_type)
{
  int      ret           = 0;
  uint8_t *fifo_buf      = NULL;
  uint32_t fifo_buf_size = 0;
  uint32_t size          = 0;

  ret = smd_get_channel_entry(ch, ch_type);

  if (ret)
    return ret;

  ch->port_info = gSMEM->smem_get_alloc_entry(
      SMEM_SMD_BASE_ID + ch->alloc_entry.cid, &size);

  fifo_buf = gSMEM->smem_get_alloc_entry(
      SMEM_SMD_FIFO_BASE_ID + ch->alloc_entry.cid, &fifo_buf_size);

  fifo_buf_size /= 2;
  ch->send_buf  = fifo_buf;
  ch->recv_buf  = fifo_buf + fifo_buf_size;
  ch->fifo_size = fifo_buf_size;

  return ret;
}

int smd_init(smd_channel_info_t *ch, uint32_t ch_type)
{
  unsigned ret        = 0;
  int      chnl_found = 0;
  uint64_t timeout    = SMD_CHANNEL_ACCESS_RETRY;

  smd_channel_alloc_entry =
      (smd_channel_alloc_entry_t *)memalign(CACHE_LINE, SMD_CHANNEL_ALLOC_MAX);
  ASSERT(smd_channel_alloc_entry);

  dprintf(INFO, "Waiting for the RPM to populate smd channel table\n");

  do {
    ret = gSMEM->smem_read_alloc_entry(
        SMEM_CHANNEL_ALLOC_TBL, (void *)smd_channel_alloc_entry,
        SMD_CHANNEL_ALLOC_MAX);
    if (ret) {
      dprintf(CRITICAL, "ERROR reading smem channel alloc tbl\n");
      return -1;
    }

    chnl_found = smd_get_channel_info(ch, ch_type);
    timeout--;
    udelay(10);
  } while (timeout && chnl_found);

  if (!timeout) {
    dprintf(CRITICAL, "Apps timed out waiting for RPM-->APPS channel entry\n");
    ASSERT(0);
  }

  register_int_handler(PcdGet64(PcdSmdIrq), smd_irq_handler, ch);

  smd_set_state(ch, SMD_SS_OPENING, 1);

  smd_notify_rpm();

  unmask_interrupt(PcdGet64(PcdSmdIrq));

  return 0;
}

void smd_uninit(smd_channel_info_t *ch)
{
  EFI_STATUS Status;
  UINTN      Index;

  Status = gBS->CreateEvent(0, 0, NULL, NULL, &smd_closed);
  ASSERT_EFI_ERROR(Status);

  smd_set_state(ch, SMD_SS_CLOSING, 1);
  smd_notify_rpm();

  /* Wait for the SMD-RPM channel to be closed */
  Status = gBS->WaitForEvent(1, &smd_closed, &Index);
  ASSERT_EFI_ERROR(Status);
}

void smd_uninit_exit_bs(smd_channel_info_t *ch)
{
  // Slightly different from previous one:
  // No event, no memory ops
  // Windows OS load time is enough for all ops
  UINTN TimeoutCount = 0;

  // Mark as exiting boot services
  smd_exit_bs_in_progress = TRUE;

  smd_set_state(ch, SMD_SS_CLOSING, 1);
  smd_notify_rpm();

  DEBUG((EFI_D_INFO, "Wait for RPM SMD channel release \n"));

  /* Wait for the SMD-RPM channel to be closed */
  while (TRUE) {
    if (smd_channel_alloc_freed)
      break;
    // 10s timeout
    if (TimeoutCount > 100000) {
      DEBUG((EFI_D_ERROR, "ERROR: RPM channel release timed out \n"));
      LibResetSystem(EfiResetCold, EFI_TIMEOUT, 0, NULL);
    }
    udelay(100);
    TimeoutCount++;
  }

  DEBUG((EFI_D_INFO, "RPM SMD Channel released \n"));
}

bool is_channel_open(smd_channel_info_t *ch)
{
  if (ch->port_info->ch0.stream_state == SMD_SS_OPENED &&
      (ch->port_info->ch1.stream_state == SMD_SS_OPENED ||
       ch->port_info->ch1.stream_state == SMD_SS_FLUSHING))
    return true;
  else
    return false;
}

/* Copy the local buffer to fifo buffer.
 * Takes care of fifo overlap.
 * Uses the fifo as circular buffer, if the request data
 * exceeds the max size of the buffer start from the beginning.
 */
static void
memcpy_to_fifo(smd_channel_info_t *ch_ptr, uint32_t *src, size_t len)
{
  uint32_t  write_index = ch_ptr->port_info->ch0.write_index;
  uint32_t *dest        = (uint32_t *)(ch_ptr->send_buf + write_index);

  while (len) {
    *dest++ = *src++;
    write_index += 4;
    len -= 4;

    if (write_index >= ch_ptr->fifo_size) {
      write_index = 0;
      dest        = (uint32_t *)(ch_ptr->send_buf + write_index);
    }
  }
  ch_ptr->port_info->ch0.write_index = write_index;
}

/* Copy the fifo buffer to a local destination.
 * Takes care of fifo overlap.
 * If the response data is split across with some part at
 * end of fifo and some at the beginning of the fifo
 */
void memcpy_from_fifo(smd_channel_info_t *ch_ptr, uint32_t *dest, size_t len)
{
  uint32_t  read_index = ch_ptr->port_info->ch1.read_index;
  uint32_t *src        = (uint32_t *)(ch_ptr->recv_buf + read_index);

  while (len) {
    *dest++ = *src++;
    read_index += 4;
    len -= 4;

    if (read_index >= ch_ptr->fifo_size) {
      read_index = 0;
      src        = (uint32_t *)(ch_ptr->recv_buf + read_index);
    }
  }

  ch_ptr->port_info->ch1.read_index = read_index;
}

void smd_read(
    smd_channel_info_t *ch, uint32_t *len, int ch_type, uint32_t *response)
{
  smd_pkt_hdr smd_hdr;
  uint32_t    size = 0;

  /* Read the indices from smem */
  ch->port_info = gSMEM->smem_get_alloc_entry(
      SMEM_SMD_BASE_ID + ch->alloc_entry.cid, &size);
  if (!ch->port_info->ch1.DTR_DSR) {
    dprintf(CRITICAL, "%s: DTR is off\n", __func__);
    ASSERT(0);
  }

  /* Wait until the data updated in the smd buffer is equal to smd packet
   * header*/
  while ((ch->port_info->ch1.write_index - ch->port_info->ch1.read_index) <
         sizeof(smd_pkt_hdr)) {
    /* Get the update info from memory */
    arch_invalidate_cache_range((addr_t)ch->port_info, size);
  }

  /* Copy the smd buffer to local buf */
  memcpy_from_fifo(ch, (uint32_t *)&smd_hdr, sizeof(smd_hdr));

  arch_invalidate_cache_range((addr_t)&smd_hdr, sizeof(smd_hdr));

  *len = smd_hdr.pkt_size;

  /* Wait on the data being updated in SMEM before returing the response */
  while ((ch->port_info->ch1.write_index - ch->port_info->ch1.read_index) <
         smd_hdr.pkt_size) {
    /* Get the update info from memory */
    arch_invalidate_cache_range((addr_t)ch->port_info, size);
  }

  /* We are good to return the response now */
  memcpy_from_fifo(ch, response, smd_hdr.pkt_size);

  arch_invalidate_cache_range((addr_t)response, smd_hdr.pkt_size);
}

void smd_signal_read_complete(smd_channel_info_t *ch, uint32_t len)
{
  /* Clear the data_written flag */
  ch->port_info->ch1.data_written = 0;

  /* Set the data_read flag */
  ch->port_info->ch0.data_read      = 1;
  ch->port_info->ch0.mask_recv_intr = 1;

  dsb();

  smd_notify_rpm();
}

int smd_write(smd_channel_info_t *ch, void *data, uint32_t len, int ch_type)
{
  smd_pkt_hdr smd_hdr;
  uint32_t    size = 0;

  memset(&smd_hdr, 0, sizeof(smd_pkt_hdr));

  if (len + sizeof(smd_hdr) > ch->fifo_size) {
    dprintf(CRITICAL, "%s: len is greater than fifo sz\n", __func__);
    return -1;
  }

  /* Read the indices from smem */
  ch->port_info = gSMEM->smem_get_alloc_entry(
      SMEM_SMD_BASE_ID + ch->alloc_entry.cid, &size);

  if (!is_channel_open(ch)) {
    dprintf(CRITICAL, "%s: channel is not in OPEN state \n", __func__);
    return -1;
  }

  if (!ch->port_info->ch0.DTR_DSR) {
    dprintf(CRITICAL, "%s: DTR is off\n", __func__);
    return -1;
  }

  /* Clear the data_read flag */
  ch->port_info->ch1.data_read = 0;

  /*copy the local buf to smd buf */
  smd_hdr.pkt_size = len;

  memcpy_to_fifo(ch, (uint32_t *)&smd_hdr, sizeof(smd_hdr));

  memcpy_to_fifo(ch, data, len);

  dsb();

  /* Set the necessary flags */

  ch->port_info->ch0.data_written   = 1;
  ch->port_info->ch0.mask_recv_intr = 0;

  dsb();

  smd_notify_rpm();

  return 0;
}

void smd_notify_rpm(void)
{
  /* Set BIT 0 to notify RPM via IPC interrupt*/
  writel(BIT(0), (UINTN)PcdGet64(PcdApcsAlias0IpcInterrupt));
}

void smd_set_state(smd_channel_info_t *ch, uint32_t state, uint32_t flag)
{
  uint32_t current_state;
  uint32_t size = 0;

  if (!ch->port_info) {
    ch->port_info = gSMEM->smem_get_alloc_entry(
        SMEM_SMD_BASE_ID + ch->alloc_entry.cid, &size);
    ASSERT(ch->port_info);
  }

  current_state = ch->port_info->ch0.stream_state;

  switch (state) {
  case SMD_SS_CLOSED:
    if (current_state == SMD_SS_OPENED) {
      smd_write_state(ch, SMD_SS_CLOSING);
    }
    else {
      smd_write_state(ch, SMD_SS_CLOSED);
    }
    break;
  case SMD_SS_OPENING:
    if (current_state == SMD_SS_CLOSING || current_state == SMD_SS_CLOSED) {
      smd_write_state(ch, SMD_SS_OPENING);
      ch->port_info->ch1.read_index     = 0;
      ch->port_info->ch0.write_index    = 0;
      ch->port_info->ch0.mask_recv_intr = 0;
    }
    break;
  case SMD_SS_OPENED:
    if (current_state == SMD_SS_OPENING) {
      smd_write_state(ch, SMD_SS_OPENED);
    }
    break;
  case SMD_SS_CLOSING:
    if (current_state == SMD_SS_OPENED) {
      smd_write_state(ch, SMD_SS_CLOSING);
    }
    break;
  case SMD_SS_FLUSHING:
  case SMD_SS_RESET:
  case SMD_SS_RESET_OPENING:
  default:
    break;
  }

  ch->current_state = state;

  smd_state_update(ch, flag);
}

enum handler_return smd_irq_handler(void *data)
{
  smd_channel_info_t *ch = (smd_channel_info_t *)data;

  if (ch->current_state == SMD_SS_CLOSED) {
    if (!smd_exit_bs_in_progress) {
      free(smd_channel_alloc_entry);
      gBS->SignalEvent(smd_closed);
    }

    smd_channel_alloc_freed = TRUE;

    return INT_NO_RESCHEDULE;
  }

  if (ch->port_info->ch1.state_updated)
    ch->port_info->ch1.state_updated = 0;

  /* Should we have to use a do while and change states until we complete */
  if (ch->current_state != ch->port_info->ch1.stream_state) {
    smd_set_state(ch, ch->port_info->ch1.stream_state, 0);
  }

  if (ch->current_state == SMD_SS_CLOSING) {
    smd_set_state(ch, SMD_SS_CLOSED, 1);
    smd_notify_rpm();
    dprintf(CRITICAL, "Channel alloc freed\n");
    smd_channel_alloc_freed = TRUE;
  }

  return INT_NO_RESCHEDULE;
}
