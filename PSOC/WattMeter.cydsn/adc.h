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
#ifndef ADC_H
#define ADC_H

extern int FwdRaw;
extern int FwdCal;
extern int FwdFilt;
extern int RefRaw;
extern int RefCal;
extern int RefFilt;
extern int Pot;
extern int VSupply;

void ADC_SAR_Seq_ISR_InterruptCallback(void);

#endif
/* [] END OF FILE */
