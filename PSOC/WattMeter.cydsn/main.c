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

volatile bool timerTick;

CY_ISR(isr_Timer_Tick)
{
    // Cleat the interrupt, this must be done in SW
    Timer_Tick_ClearInterrupt(Timer_Tick_INTR_MASK_TC);
    timerTick = true;
}

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    Opamp_Fwd_Start();
    Opamp_Ref_Start();
    ADC_SAR_Seq_Start();
    ADC_SAR_Seq_StartConvert();
    ADC_SAR_Seq_IRQ_Enable();
    IDAC8_Start();
    
    isr_Timer_Tick_StartEx(isr_Timer_Tick);
    isr_Timer_Tick_Enable();
    Timer_Tick_Start();
    
    for(;;)
    {
        if(timerTick)
        {
            timerTick = false;
            Control_Reg_Control ^= 1;
        }
        // Every X ticks
        // Calculate SWR
        // Run High SWR output
        // Run Meter output
        
        // Update display
        
    }
}

/* [] END OF FILE */
