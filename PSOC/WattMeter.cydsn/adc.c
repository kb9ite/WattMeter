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
#include "adc.h"
#include "project.h"
#include "cal.h"

int FwdRaw;
power32b16 FwdCal;
power32b16 FwdFilt;

int RefRaw;
power32b16 RefCal;
power32b16 RefFilt;

int FiltUp = 100;
int FiltDown = 100;

void ADC_SAR_Seq_ISR_InterruptCallback(void)
{
    Output_Reg_Control |= 0x10;
    
    // Read the inputs and calibrate them
    FwdRaw = ADC_SAR_Seq_GetResult16(0);
    FwdCal = FwdPowerCal[FwdRaw & 0x0FFF];
    RefRaw = ADC_SAR_Seq_GetResult16(1);
    RefCal = RefPowerCal[RefRaw & 0x0FFF];
    
    
    // Compute the filtered powers    
    int filt = FwdCal > FwdFilt ? FiltUp : FiltDown;
    FwdFilt = (((int64)FwdCal * filt) + ((int64)FwdFilt * (65536 - filt))) >> 16;
    
    filt = RefCal > RefFilt ? FiltUp : FiltDown;
    RefFilt = (((int64)RefCal * filt) + ((int64)RefFilt * (65536 - filt))) >> 16;

    Output_Reg_Control &= 0xEF;
}

/* [] END OF FILE */
