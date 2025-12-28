void npu_setup_irq(int, void *);

#define	LL_ATON_OSAL_INIT()
#define	LL_ATON_OSAL_DEINIT()
#define	LL_ATON_OSAL_WFE()		__WFE()
#define	LL_ATON_OSAL_SIGNAL_EVENT()
#define	LL_ATON_OSAL_INSTALL_IRQ(irq_aton_line_nr, handler)	\
    npu_setup_irq(irq_aton_line_nr, handler)
