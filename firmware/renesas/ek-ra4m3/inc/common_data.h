/* generated common header file - do not edit */
#ifndef COMMON_DATA_H_
#define COMMON_DATA_H_
#include "bsp_api.h"
#include "bsp_pin_cfg.h"
#include "r_ioport.h"
#include <stdint.h>
FSP_HEADER
#define IOPORT_CFG_NAME g_bsp_pin_cfg

/* IOPORT Instance */
extern const ioport_instance_t g_ioport;

/* IOPORT control structure. */
extern ioport_instance_ctrl_t g_ioport_ctrl;
void g_common_init(void);
FSP_FOOTER
#endif /* COMMON_DATA_H_ */
