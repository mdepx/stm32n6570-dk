#include <sys/systm.h>

void npu_setup_irq(int, void *);

#define	LL_ATON_OSAL_ENTER_CS()		critical_enter();
#define	LL_ATON_OSAL_EXIT_CS()		critical_exit();
#define	LL_ATON_OSAL_INIT()
#define	LL_ATON_OSAL_DEINIT()
#define	LL_ATON_OSAL_WFE()		__WFE()
#define	LL_ATON_OSAL_SIGNAL_EVENT()
#define	LL_ATON_OSAL_INSTALL_IRQ(irq_aton_line_nr, handler)	\
    npu_setup_irq(irq_aton_line_nr, handler)
#define	LL_ATON_OSAL_LOCK_NPU_CACHE()
#define	LL_ATON_OSAL_UNLOCK_NPU_CACHE()
