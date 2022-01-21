#ifndef _MY_PRINT_H_
#define _MY_PRINT_H_

#if 1
#define MY_PRINT printf
#define MY_DEBUG_TX debug_uart_tx_bytes
#else
#define MY_PRINT    //printf
#define MY_DEBUG_TX //debug_uart_tx_bytes
#endif


#endif