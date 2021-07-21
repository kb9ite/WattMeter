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

#include "types.h"
    
extern int FwdRaw;
extern power32b16 FwdCal;
extern power32b16 FwdFilt;
extern int RefRaw;
extern power32b16 RefCal;
extern power32b16 RefFilt;

extern int FiltUp;
extern int FiltDown;
    
void ADC_SAR_Seq_ISR_InterruptCallback(void);

#endif
/* [] END OF FILE */
