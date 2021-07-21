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

#ifndef TYPES_H_
#define TYPES_H_

#define MAX(a, b) ((a > b) ? (a) : (b))
#define MIN(a, b) ((a < b) ? (a) : (b))
    
#define WATT2INTPOWER(x) ((int)(x * 65536))
#define SWR2INTSWR(x)    ((int)(x * 65536))
    
typedef int power32b16;
typedef int swr32b16;
    
#endif
/* [] END OF FILE */
