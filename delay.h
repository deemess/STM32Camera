#ifndef DELAY_H
#define DELAY_H

//#include <stdint.h>

#define F_CPU 72000000

/*
inline void delay_loops(uint32_t loops) __attribute__ ((always_inline));
inline void delay_loops(uint32_t loops) {
   asm volatile (
      "1: \n"
      " SUBS %[loops], %[loops], #1 \n"
      " BNE 1b \n"
         : [loops] "+r"(loops)
   );
}
*/

inline void delay_loops(uint32_t loops);

#define _delay_us( US ) delay_loops( (uint32_t)((double)US * F_CPU / 3000000.0) )
#define _delay_ms( MS ) delay_loops( (uint32_t)((double)MS * F_CPU / 3000.0) )
#define _delay_s( S )   delay_loops( (uint32_t)((double)S  * F_CPU / 3.0) )


#endif
