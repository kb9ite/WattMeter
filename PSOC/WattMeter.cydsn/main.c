/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <stdbool.h>
#include "u8g2Support.h"

volatile bool timerTick;
u8g2_t u8g2;

CY_ISR(isr_Timer_Tick)
{
    // Clear the interrupt, this must be done in SW
    Timer_Tick_ClearInterrupt(Timer_Tick_INTR_MASK_TC);
    timerTick = true;
}

uint8_t tickCnt;
const uint32 test = 0xAA00FF55;
extern uint8 sendZeros;

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    // Startup analog components
    Opamp_Fwd_Start();
    Opamp_Ref_Start();
    ADC_SAR_Seq_Start();
    ADC_SAR_Seq_StartConvert();
    ADC_SAR_Seq_IRQ_Enable();
    IDAC8_Start();
    
    //Startup display
    SPI_Start();
    UART_Start();
    u8g2_Setup_sh1106_128x64_noname_f(&u8g2, U8G2_R0, u8x8_hw_spi, u8x8_gpio_and_delay);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    
    // Startup timer last
    isr_Timer_Tick_StartEx(isr_Timer_Tick);
    isr_Timer_Tick_Enable();
    Timer_Tick_Start();
    
    for(;;)
    {
        if(timerTick)
        {
            timerTick = false;
            Control_Reg_Control ^= 1;
            if(tickCnt == 0)
            {
                u8g2_ClearBuffer(&u8g2);
                u8g2_ClearDisplay(&u8g2);
                u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
                u8g2_DrawStr(&u8g2, 0, 20, "Hello World");
                u8g2_DrawLine(&u8g2, 20, 40, 120, 40);
                u8g2_DrawBox(&u8g2, 50, 20, 25, 25);
                u8g2_SendBuffer(&u8g2);
                
                sendZeros = !sendZeros;
            }
            tickCnt++;
            //SPI_SpiUartPutArray(&test, 4);
        }
        // Every X ticks
        // Calculate SWR
        // Run High SWR output
        // Run Meter output
        
        // Update display
        
    }
}

/* [] END OF FILE */
