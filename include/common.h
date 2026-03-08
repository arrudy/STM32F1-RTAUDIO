#ifndef COMMON_H
#define COMMON_H

#include <zephyr/kernel.h>

// Helper: This function is called automatically when the 'key' variable goes out of scope
static inline void __irq_unlock_scope(unsigned int *key) {
    irq_unlock(*key);
}

/**
 * @brief Scoped IRQ Lock
 * Creates a block where interrupts are disabled. 
 * Automatically re-enables them at the end of the { } block.
 */
#define IRQ_LOCKED \
    for (unsigned int __key __attribute__((cleanup(__irq_unlock_scope))) = irq_lock(), \
         __once = 1; __once; __once = 0)



#endif