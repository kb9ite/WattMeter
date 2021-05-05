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
int FwdCal;
int FwdFilt;
int RefRaw;
int RefCal;
int RefFilt;
int Pot;
int VSupply;

int Debug_SampleCount;

void ADC_SAR_Seq_ISR_InterruptCallback(void)
{
    Control_Reg_Control ^= 4;
    Control_Reg_Control |= 2;
    // Read the inputs and calibrate them
    FwdRaw = ADC_SAR_Seq_GetResult16(0);
    FwdCal = PowerCal[FwdRaw & 0x0FFF];
    RefRaw = ADC_SAR_Seq_GetResult16(1);
    RefCal = PowerCal[RefRaw & 0x0FFF];
    Pot = (ADC_SAR_Seq_GetResult16(2) >> 4) & 0x000000FF;
    VSupply = ADC_SAR_Seq_GetResult16(3);
    
    // Compute the filtered powers    
    int filt = FwdCal > FwdFilt ? FiltUpCal[Pot] : FiltDownCal[Pot];
    FwdFilt = ((FwdCal * filt) + (FwdFilt * (65535 - filt))) >> 18;
    
    filt = RefCal > RefFilt ? FiltUpCal[Pot] : FiltDownCal[Pot];
    RefFilt = ((RefCal * filt) + (RefFilt * (65535 - filt))) >> 18;
    
    Debug_SampleCount++;
    Control_Reg_Control &= 0xFD;
}

/* [] END OF FILE */
