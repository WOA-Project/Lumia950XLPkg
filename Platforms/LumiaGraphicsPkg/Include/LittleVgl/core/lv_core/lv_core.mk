CSRCS += lv_group.c
CSRCS += lv_indev.c
CSRCS += lv_obj.c
CSRCS += lv_refr.c
CSRCS += lv_style.c
CSRCS += lv_vdb.c

DEPPATH += --dep-path lvgl/lv_core
VPATH += :lvgl/lv_core

CFLAGS += "-I$(LVGL_DIR)/lvgl/lv_core"
