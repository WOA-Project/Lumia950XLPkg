/* Copyright (c) 2012, 2014-2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
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
#include <Library/MallocLib.h>
#include <Protocol/QcomSpmi.h>

#include "spmi_p.h"

#define SPMI_BASE ((UINTN)PcdGet64(PcdSpmiBaseAddress))
#define PMIC_ARB_CORE ((UINTN)PcdGet64(PcdPmicArbCoreAddress))

#define PMIC_ARB_V2 0x20010000
#define CHNL_IDX(sid, pid) ((sid << 8) | pid)

static uint32_t pmic_arb_chnl_num;
static uint32_t pmic_arb_owner_id;
static uint32_t pmic_arb_ver;
static uint8_t *chnl_tbl;
static uint32_t max_peripherals;
static uint32_t spmi_version = 0;

static void spmi_lookup_chnl_number(void)
{
	uint32_t i;
	uint8_t slave_id = 0;
	uint8_t ppid_address = 0;
	/* We need a max of sid (4 bits) + pid (8bits) of uint8_t's */
	uint32_t chnl_tbl_sz = BIT(12) * sizeof(uint8_t);

	/* Allocate the channel table */
	chnl_tbl = (uint8_t *) malloc(chnl_tbl_sz);
	ASSERT(chnl_tbl);

	for(i = 0; i < max_peripherals; i++)
	{
		if (spmi_version==2) {
			slave_id = (readl(PMICV2_ARB_REG_CHLN(i)) & 0xf0000) >> 16;
			ppid_address = (readl(PMICV2_ARB_REG_CHLN(i)) & 0xff00) >> 8;
		}

		chnl_tbl[CHNL_IDX(slave_id, ppid_address)] = i;
	}
}

/* Function to initialize SPMI controller.
 * chnl_num : Channel number to be used by this EE.
 */
int spmi_init(void)
{
	spmi_version = PcdGet64(PcdSpmiVersion);
	if (spmi_version > 2) {
		DEBUG((EFI_D_ERROR, "Invalid SPMI version: %u\n", spmi_version));
		return -1;
	}

	if (SPMI_BASE==0) {
		DEBUG((EFI_D_ERROR, "PcdSpmiBaseAddress is required for SPMI\n"));
		return -1;
	}

	if (spmi_version==2 && PMIC_ARB_CORE==0) {
		DEBUG((EFI_D_ERROR, "PcdPmicArbCoreAddress is required for SPMIV2\n"));
		return -1;
	}

	/* Read the version numver */
	pmic_arb_ver = readl(PMIC_ARB_SPMI_HW_VERSION);
	max_peripherals = PcdGet64(PcdSpmiMaxPeripherals);

	if (pmic_arb_ver < PMIC_ARB_V2)
	{
		/* Initialize PMIC Arbiter Channel Number to
		 * 0 by default of V1 HW
		 */
		pmic_arb_chnl_num = PcdGet64(PcdPmicArbChannelNum);
		pmic_arb_owner_id = PcdGet64(PcdPmicArbOwnerId);
	}
	else
	{
		spmi_lookup_chnl_number();
	}

	return 0;
}

static void write_wdata_from_array(uint8_t *array,
	                               uint8_t reg_num,
	                               uint8_t array_size,
	                               uint8_t* bytes_written)
{
	uint32_t shift_value[] = {0, 8, 16, 24};
	int i;
	uint32_t val = 0;

	/* Write to WDATA */
	for (i = 0; (*bytes_written < array_size) && (i < 4); i++)
	{
		val |= (uint32_t)(array[*bytes_written]) << shift_value[i];
		(*bytes_written)++;
	}

	if (spmi_version==2)
		writel(val, PMICV2_ARB_CHNLn_WDATA(pmic_arb_chnl_num, reg_num));
	else
		writel(val, PMICV1_ARB_CHNLn_WDATA(pmic_arb_chnl_num, reg_num));
}

/* Initiate a write cmd by writing to cmd register.
 * Commands are written according to cmd parameters
 * cmd->opcode   : SPMI opcode for the command
 * cmd->priority : Priority of the command
 *                 High priority : 1
 *                 Low Priority : 0
 * cmd->address  : SPMI Peripheral Address.
 * cmd->offset   : Offset Address for the command.
 * cmd->bytecnt  : Number of bytes to be written.
 *
 * param is the parameter to the command
 * param->buffer : Value to be written
 * param->size   : Size of the buffer.
 *
 * return value : 0 if success, the error bit set on error
 */
unsigned int pmic_arb_write_cmd(struct pmic_arb_cmd *cmd,
                                struct pmic_arb_param *param)
{
	uint32_t bytes_written = 0;
	uint32_t error;
	uint32_t val = 0;

	/* Look up for pmic channel only for V2 hardware
	 * For V1-HW we dont care for channel number & always
	 * use '0'
	 */
	if (pmic_arb_ver >= PMIC_ARB_V2)
	{
		pmic_arb_chnl_num = chnl_tbl[CHNL_IDX(cmd->slave_id, cmd->address)];
	}

	/* Disable IRQ mode for the current channel*/
	if (spmi_version==2)
		writel(0x0, PMICV2_ARB_CHNLn_CONFIG(pmic_arb_chnl_num));
	else
		writel(0x0, PMICV1_ARB_CHNLn_CONFIG(pmic_arb_chnl_num));
	/* Write parameters for the cmd */
	if (cmd == NULL)
	{
		dprintf(CRITICAL,"PMIC arbiter error, no command provided\n");
		return 1;
	}

	/* Write the data bytes according to the param->size
	 * Can write upto 8 bytes.
	 */

	/* Write first 4 bytes to WDATA0 */
	write_wdata_from_array(param->buffer, 0, param->size,(uint8_t *)&bytes_written);

	if (bytes_written < param->size)
	{
		/* Write next 4 bytes to WDATA1 */
		write_wdata_from_array(param->buffer, 1, param->size, (uint8_t *)&bytes_written);
	}

	/* Fill in the byte count for the command
	 * Note: Byte count is one less than the number of bytes transferred.
	 */
	cmd->byte_cnt = param->size - 1;
	/* Fill in the Write cmd opcode. */
	cmd->opcode = SPMI_CMD_EXT_REG_WRTIE_LONG;

	/* Write the command */
	val = 0;
	val |= ((uint32_t)(cmd->opcode) << PMIC_ARB_CMD_OPCODE_SHIFT);
	val |= ((uint32_t)(cmd->priority) << PMIC_ARB_CMD_PRIORITY_SHIFT);
	if (spmi_version!=2) {
		val |= ((uint32_t)(cmd->slave_id) << PMIC_ARB_CMD_SLAVE_ID_SHIFT);
		val |= ((uint32_t)(cmd->address) << PMIC_ARB_CMD_ADDR_SHIFT);
	}
	val |= ((uint32_t)(cmd->offset) << PMIC_ARB_CMD_ADDR_OFFSET_SHIFT);
	val |= ((uint32_t)(cmd->byte_cnt));

	if (spmi_version==2)
		writel(val, PMICV2_ARB_CHNLn_CMD0(pmic_arb_chnl_num));
	else
		writel(val, PMICV1_ARB_CHNLn_CMD0(pmic_arb_chnl_num));

	/* Wait till CMD DONE status */
	if (spmi_version==2)
		while (!(val = readl(PMICV2_ARB_CHNLn_STATUS(pmic_arb_chnl_num))));
	else
		while (!(val = readl(PMICV1_ARB_CHNLn_STATUS(pmic_arb_chnl_num))));

	/* Check for errors */
	error = val ^ (1 << PMIC_ARB_CMD_DONE);
	if (error)
	{
		dprintf(CRITICAL, "SPMI write command failure: \
			cmd_id = %u, error = %u\n", cmd->opcode, error);
		return error;
	}
	else
		return 0;
}

static void read_rdata_into_array(uint8_t *array,
                                  uint8_t reg_num,
                                  uint8_t array_size,
                                  uint8_t* bytes_read)
{
	uint32_t val = 0;
	uint32_t mask_value[] = {0xFF, 0xFF00, 0xFF0000, 0xFF000000};
	uint8_t shift_value[] = {0, 8, 16, 24};
	int i;

	if (spmi_version==2)
		val = readl(PMICV2_ARB_OBS_CHNLn_RDATA(pmic_arb_chnl_num, reg_num));
	else
		val = readl(PMICV1_ARB_CHNLn_RDATA(pmic_arb_chnl_num, reg_num));

	/* Read at most 4 bytes */
	for (i = 0; (i < 4) && (*bytes_read < array_size); i++)
	{
		array[*bytes_read] = (val & mask_value[i]) >> shift_value[i];
		(*bytes_read)++;
	}
}

/* Initiate a read cmd by writing to cmd register.
 * Commands are written according to cmd parameters
 * cmd->opcode   : SPMI opcode for the command
 * cmd->priority : Priority of the command
 *                 High priority : 1
 *                 Low Priority : 0
 * cmd->address  : SPMI Peripheral Address.
 * cmd->offset   : Offset Address for the command.
 * cmd->bytecnt  : Number of bytes to be read.
 *
 * param is the buffer to the save command data.
 * param->buffer : Buffer to store the bytes returned.
 * param->size   : Size of the buffer.
 *
 * return value : 0 if success, the error bit set on error
 */
unsigned int pmic_arb_read_cmd(struct pmic_arb_cmd *cmd,
                               struct pmic_arb_param *param)
{
	uint32_t val = 0;
	uint32_t error;
	uint8_t bytes_read = 0;

	/* Look up for pmic channel only for V2 hardware
	 * For V1-HW we dont care for channel number & always
	 * use '0'
	 */
	if (pmic_arb_ver >= PMIC_ARB_V2)
	{
		pmic_arb_chnl_num = chnl_tbl[CHNL_IDX(cmd->slave_id, cmd->address)];
	}


	/* Disable IRQ mode for the current channel*/
	if (spmi_version==2)
		writel(0x0, PMICV2_ARB_OBS_CHNLn_CONFIG(pmic_arb_chnl_num));
	else
		writel(0x0, PMICV1_ARB_CHNLn_CONFIG(pmic_arb_chnl_num));

	/* Fill in the byte count for the command
	 * Note: Byte count is one less than the number of bytes transferred.
	 */
	cmd->byte_cnt = param->size - 1;
	/* Fill in the Write cmd opcode. */
	cmd->opcode = SPMI_CMD_EXT_REG_READ_LONG;

	val |= ((uint32_t)(cmd->opcode) << PMIC_ARB_CMD_OPCODE_SHIFT);
	val |= ((uint32_t)(cmd->priority) << PMIC_ARB_CMD_PRIORITY_SHIFT);
	if (spmi_version!=2) {
		val |= ((uint32_t)(cmd->slave_id) << PMIC_ARB_CMD_SLAVE_ID_SHIFT);
		val |= ((uint32_t)(cmd->address) << PMIC_ARB_CMD_ADDR_SHIFT);
	}
	val |= ((uint32_t)(cmd->offset) << PMIC_ARB_CMD_ADDR_OFFSET_SHIFT);
	val |= ((uint32_t)(cmd->byte_cnt));

	if (spmi_version==2)
		writel(val, PMICV2_ARB_OBS_CHNLn_CMD0(pmic_arb_chnl_num));
	else
		writel(val, PMICV1_ARB_CHNLn_CMD0(pmic_arb_chnl_num));

	/* Wait till CMD DONE status */
	if (spmi_version==2)
		while (!(val = readl(PMICV2_ARB_OBS_CHNLn_STATUS(pmic_arb_chnl_num))));
	else
		while (!(val = readl(PMICV1_ARB_CHNLn_STATUS(pmic_arb_chnl_num))));

	/* Check for errors */
	error = val ^ (1 << PMIC_ARB_CMD_DONE);

	if (error)
	{
		dprintf(CRITICAL, "SPMI read command failure: \
			cmd_id = %u, error = %u\n", cmd->opcode, error);
		return error;
	}

	/* Read the RDATA0 */
	read_rdata_into_array(param->buffer, 0, param->size , &bytes_read);

	if (bytes_read < param->size)
	{
		/* Read the RDATA1 */
		read_rdata_into_array(param->buffer, 1, param->size , &bytes_read);

	}

	if (bytes_read < param->size)
	{
		/* Read the RDATA2 */
		read_rdata_into_array(param->buffer, 2, param->size , &bytes_read);

	}

	return 0;
}
