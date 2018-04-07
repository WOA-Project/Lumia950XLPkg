#ifndef __QCOM_PROTOCOL_GPIO_TLMM_INTERRUPT_H__
#define __QCOM_PROTOCOL_GPIO_TLMM_INTERRUPT_H__

#define QCOM_GPIO_TLMM_INTERRUPT_PROTOCOL_GUID \
  { 0x1634c987, 0x50a7, 0x4f98, { 0x88, 0xf0, 0x7e, 0xbc, 0x60, 0x11, 0xa5, 0x32 } }

typedef struct _QCOM_GPIO_TLMM_INTERRUPT_PROTOCOL   QCOM_GPIO_TLMM_INTERRUPT_PROTOCOL;

#define TLMM_IRQ_TYPE_NONE          0x00000000
#define TLMM_IRQ_TYPE_EDGE_RISING   0x00000001
#define TLMM_IRQ_TYPE_EDGE_FALLING  0x00000002
#define TLMM_IRQ_TYPE_EDGE_BOTH     (TLMM_IRQ_TYPE_EDGE_FALLING | TLMM_IRQ_TYPE_EDGE_RISING)
#define TLMM_IRQ_TYPE_LEVEL_HIGH    0x00000004
#define TLMM_IRQ_TYPE_LEVEL_LOW     0x00000008
#define TLMM_IRQ_TYPE_LEVEL_MASK    (TLMM_IRQ_TYPE_LEVEL_LOW | TLMM_IRQ_TYPE_LEVEL_HIGH)
#define TLMM_IRQ_TYPE_SENSE_MASK    0x0000000f

typedef VOID (*GPIO_INTERRUPT_HANDLER)(UINTN Id);

typedef VOID (EFIAPI *gpio_tlmm_irq_mask_t)(UINTN id);
typedef VOID (EFIAPI *gpio_tlmm_irq_unmask_t)(UINTN id);
typedef VOID (EFIAPI *gpio_tlmm_irq_ack_t)(UINTN id);
typedef EFI_STATUS (EFIAPI *gpio_tlmm_irq_set_type_t)(UINTN id, UINTN type);
typedef VOID (EFIAPI *gpio_tlmm_irq_set_custom_handler_t)(UINTN id, GPIO_INTERRUPT_HANDLER handler);

struct _QCOM_GPIO_TLMM_INTERRUPT_PROTOCOL {
  gpio_tlmm_irq_mask_t                IrqMask;
  gpio_tlmm_irq_unmask_t              IrqUnMask;
  gpio_tlmm_irq_ack_t                 IrqAck;
  gpio_tlmm_irq_set_type_t            IrqSetType;
  gpio_tlmm_irq_set_custom_handler_t  IrqSetCustomHandler;
};

extern EFI_GUID gQcomGpioTlmmInterruptProtocolGuid;

#endif
