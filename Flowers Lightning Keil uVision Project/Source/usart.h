#ifndef USART_H_
#define USART_H_

#include "stm32f10x_usart.h"

#define RX_BUF_SIZE 80

class USART
{
public:
    static unsigned char RX_FLAG_END_LINE;
    static unsigned char RXi;
    static unsigned char RXc;
    static unsigned char RX_BUF[RX_BUF_SIZE];
    static unsigned char buffer[80];
    
	static void Init();
    static void USARTSend(const unsigned char *pucBuffer);
    static void Clear_RXBuffer();
};

#endif
