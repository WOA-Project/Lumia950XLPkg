#ifndef __QCOM_PROTOCOL_CLOCK_H__
#define __QCOM_PROTOCOL_CLOCK_H__

#include <Chipset/clock.h>

#define QCOM_CLOCK_PROTOCOL_GUID \
  { 0x4fcc91c2, 0x9c4f, 0x4e3c, { 0xa6, 0x73, 0xc6, 0xdf, 0x62, 0xe0, 0x41, 0xd5 } }

typedef struct _QCOM_CLOCK_PROTOCOL   QCOM_CLOCK_PROTOCOL;

typedef struct clk *(EFIAPI *clk_get_t)(const char *id);
typedef int (EFIAPI *clk_enable_t)(struct clk *clk);
typedef void (EFIAPI *clk_disable_t)(struct clk *clk);
typedef unsigned long (EFIAPI *clk_get_rate_t)(struct clk *clk);
typedef int (EFIAPI *clk_set_rate_t)(struct clk *clk, unsigned long rate);
typedef int (EFIAPI *clk_set_parent_t)(struct clk *clk, struct clk *parent);
typedef struct clk *(EFIAPI *clk_get_parent_t)(struct clk *clk);
typedef int (EFIAPI *clk_get_set_enable_t)(char *id, unsigned long rate, bool enable);
typedef int (EFIAPI *clk_reset_t)(struct clk *clk, enum clk_reset_action);

struct _QCOM_CLOCK_PROTOCOL {
  clk_get_t             clk_get;
  clk_enable_t          clk_enable;
  clk_disable_t         clk_disable;
  clk_get_rate_t        clk_get_rate;
  clk_set_rate_t        clk_set_rate;
  clk_set_parent_t      clk_set_parent;
  clk_get_parent_t      clk_get_parent;
  clk_get_set_enable_t  clk_get_set_enable;
  clk_reset_t           clk_reset;
};

extern EFI_GUID gQcomClockProtocolGuid;

#endif
