#ifndef NELIBC_SYS_REBOOT_WRAPPER_H
#define NELIBC_SYS_REBOOT_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <linux/reboot.h>

#define RB_AUTOBOOT LINUX_REBOOT_CMD_RESTART
#define RB_HALT_SYSTEM LINUX_REBOOT_CMD_HALT
#define RB_ENABLE_CAD LINUX_REBOOT_CMD_CAD_ON
#define RB_DISABLE_CAD LINUX_REBOOT_CMD_CAD_OFF
#define RB_POWER_OFF LINUX_REBOOT_CMD_POWER_OFF
#define RB_SW_SUSPEND LINUX_REBOOT_CMD_SW_SUSPEND
#define RB_KEXEC LINUX_REBOOT_CMD_KEXEC

#ifdef __cplusplus
extern "C" {
#endif
int reboot(int command);
#ifdef __cplusplus
}
#endif

#else
#include_next <sys/reboot.h>
#endif

#endif
