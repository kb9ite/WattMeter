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
#include <stdio.h>
#include "adc.h"
#include <math.h>
#include "meter.h"

#define SCREEN_TOP    (2)
#define SCREEN_LEFT   (4) 
#define FONT_HEIGHT   (10)
#define SCREEN_WIDTH  (128)
#define SCREEN_HEIGHT (64)
#define BAR_HEIGHT    (10)
#define MARGIN        (3)

#define DISPLAYDOWNSAMPLE 3

#define MIN_SWR_POWER  WATT2INTPOWER(5)
#define WARN_SWR_LEVEL SWR2INTSWR(2)

typedef enum
{
    SWITCH_200,
    SWITCH_2000,
    SWITCH_SWR,
    SWITCH_ERROR,
}E_Switch;

typedef enum
{
    FILT_SLOW,
    FILT_MED,
    FILT_FAST,
    FILT_PEAKFAST,
    FILT_PEAKMED,
    FILT_PEAKSLOW,
    FILT_ERROR,
}E_FiltMode;

bool timerTick;
u8g2_t u8g2;
int displayDownsampleCounter = 0;
char tempStr[20];

bool fwdHighRange = false;
bool highSWR = false;
int pot;
E_Switch switchState;
E_FiltMode filtMode;
swr32b16 swr = SWR2INTSWR(1);

void ReadInputs(void);
void SetFilterConstants(void);
void UpdateScreen(void);

CY_ISR(isr_Timer_Tick)
{
    // Clear the interrupt, this must be done in SW
    Timer_Tick_ClearInterrupt(Timer_Tick_INTR_MASK_TC);
    timerTick = true;
}


int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    // Startup analog components
    Opamp_FwdBuf_Start();
    Opamp_RefBuf_Start();
    Opamp_FwdFilt_Start();
    Opamp_RefFilt_Start();
    
    ADC_SAR_Seq_Start();
    ADC_SAR_Seq_StartConvert();
    ADC_SAR_Seq_IRQ_Enable();
    IDAC8_Start();
    
    //Startup display
    SPI_Start();
    u8g2_Setup_sh1106_128x64_noname_f(&u8g2, U8G2_R0, u8x8_hw_spi, u8x8_gpio_and_delay);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_SetFont(&u8g2, u8g2_font_helvR10_tr);
    
    //Startup the components for measuring the pot
    LPComp_1_Start();
    Timer_1_Start();
    
    // Startup the timer last
    isr_Timer_Tick_StartEx(isr_Timer_Tick);
    isr_Timer_Tick_Enable();
    Timer_Tick_Start();
    
    for(;;)
    {
        Output_Reg_Control ^= 0x20;
        if(timerTick)
        {
            // Clear flag for next tick
            timerTick = false;

            // Read the potentiometer
            ReadInputs();
            SetFilterConstants(); 
                    
            // swr = (1 + sqrt(ref/fwd)) / (1 - sqrt(ref/fwd))
            if(FwdFilt >= MIN_SWR_POWER)
            {
                float ratio = sqrtf((float)RefFilt / (float)FwdFilt);
                volatile float swrFloat = (1 + ratio) / (1 - ratio);
                swr = (int)(swrFloat * 65536);
            }
            // Run DAC output based on switch input
            switch(switchState)
            {
                case SWITCH_200:
                    Meter_SetPower200(FwdFilt);
                    break;
                case SWITCH_2000:
                    Meter_SetPower2000(FwdFilt);
                    break;
                case SWITCH_SWR:
                    Meter_SetSWR(swr);
                    break;
                case SWITCH_ERROR:
                    Meter_SetPower200(0);
                    break;
            }

            // Perform high SWR check
            highSWR = swr > WARN_SWR_LEVEL;
            fwdHighRange |= FwdFilt > WATT2INTPOWER(150);
            
            // Update the screen every downsample ticks
            if(displayDownsampleCounter == 0)
            {
                displayDownsampleCounter = DISPLAYDOWNSAMPLE;
                UpdateScreen();
            }
            displayDownsampleCounter--;
        }
    }
}

void ReadInputs()
{
    /***** Read the pot input *****/
    // This indicates the phase of reading the pot, fals = discharging , true = charging
    static bool potReadPhase = false;
    if(potReadPhase)
    {// charge
        // Enable the counter, remove reset from the counter and turn off the pulldown.
        Control_Reg_1_Control = 1;                
    }
    else
    {
        // if the time had a successful capture
        if((Timer_1_ReadStatusRegister() & 0x2) != 0)
        {
            // Get the captured timer value
            int newValue = 255 - (Timer_1_ReadCapture() >> 8);
            // Apply some filtering
            pot = ((newValue * 1) + (pot * 3)) >> 2;
        }
        // discharge and reset
        Control_Reg_1_Control = 2;
        // Clear the capture FIFO in case there were multiple captures.
        Timer_1_ClearFIFO();
    }
    // Change to the next phase
    potReadPhase = !potReadPhase;

    /***** Read the switch input *****/
    if((Inputs_Reg_Status & 0x01) == 0)
    {
        switchState = SWITCH_200;
    }
    else if((Inputs_Reg_Status & 0x02) == 0)
    {
        switchState = SWITCH_2000;
    }
    else if((Inputs_Reg_Status & 0x04) == 0)
    {
        switchState = SWITCH_SWR;
    }
    else
    {
        switchState = SWITCH_ERROR;
    }
}

void SetFilterConstants(void)
{
    switch(filtMode)
    {
        case FILT_SLOW:
            FiltUp = 2;
            FiltDown = 2;
        
            if(pot > 40)
            {
                filtMode = FILT_MED;
            }
            break;
        case FILT_MED:
            FiltUp = 7;
            FiltDown = 7;
            
            if(pot < 30)
            {
                filtMode = FILT_SLOW;
            }
            else if(pot > 70)
            {
                filtMode = FILT_FAST;
            }
            break;
        case FILT_FAST:
            FiltUp = 12;
            FiltDown = 12;
            if(pot < 60)
            {
                filtMode = FILT_MED;
            }
            else if(pot > 115)
            {
                filtMode = FILT_PEAKFAST;
            }
            break;
        case FILT_PEAKFAST:
            FiltUp = 10000;
            FiltDown = 12;
            if(pot < 105)
            {
                filtMode = FILT_FAST;
            }
            else if(pot > 140)
            {
                filtMode = FILT_PEAKMED;
            }
            break;
        case FILT_PEAKMED:
            FiltUp = 10000;
            FiltDown = 7;
            
            if(pot < 130)
            {
                filtMode = FILT_PEAKFAST;
            }
            else if(pot > 175)
            {
                filtMode = FILT_PEAKSLOW;
            }
            break;
        case FILT_PEAKSLOW:
            FiltUp = 10000;
            FiltDown = 2;
            
            if(pot < 165)
            {
                filtMode = FILT_PEAKMED;
            }
            break;        
        default:
            filtMode = FILT_MED;
            break;
    }            
}

void PrintNumber(char* str, int size, char* prefix, int value)
{
    if(value < (10 << 16))
    {
        snprintf(str, size, "%s%d.%02d", prefix, 
            value >> 16, ((value & 0x0000FFFF) * 100) >> 16);
    }
    else if(value < (100 << 16))
    {
        snprintf(str, size, "%s%d.%01d", prefix, 
            value >> 16, ((value & 0x0000FFFF) * 10) >> 16);
    }
    else
    {
        snprintf(str, size, "%s%d", prefix, 
            value >> 16);
    }
}
int DrawElement(char* name, int value, int barMin, int barMax, int top)
{
    PrintNumber(tempStr, sizeof(tempStr), name, value);
    u8g2_DrawStr(&u8g2, SCREEN_LEFT, top + FONT_HEIGHT, tempStr);
    
    top += FONT_HEIGHT + MARGIN;
    
    u8g2_DrawFrame(&u8g2, SCREEN_LEFT, top, SCREEN_WIDTH - SCREEN_LEFT*2, BAR_HEIGHT);
    
    int fillWidth = 0;
    if(value > barMin)
    {
        fillWidth = (SCREEN_WIDTH - SCREEN_LEFT*2) * (value - barMin) / (barMax - barMin);
        if(fillWidth > (SCREEN_WIDTH - SCREEN_LEFT))
        {
            fillWidth = SCREEN_WIDTH - SCREEN_LEFT;
        }
    }
    u8g2_DrawBox(&u8g2, SCREEN_LEFT, top+1, fillWidth, BAR_HEIGHT-2);
    
    top += BAR_HEIGHT;
    return top;
}

void UpdateScreen(void)
{
#ifdef DEBUGSCREEN
    // Clear the display
    u8g2_ClearBuffer(&u8g2);

    snprintf(tempStr, sizeof(tempStr), "F %d %d.%02d %d.%02d", FwdRaw, FwdCal>>16, ((FwdCal & 0x0000FFFF) * 100) >> 16, FwdFilt >> 16, ((FwdFilt & 0x0000FFFF) * 100) >> 16);
    u8g2_DrawStr(&u8g2, 2, 11, tempStr);

    snprintf(tempStr, sizeof(tempStr), "R %d %d.%02d %d.%02d", RefRaw, RefCal>>16, ((RefCal & 0x0000FFFF) * 100) >> 16, RefFilt >> 16, ((RefFilt & 0x0000FFFF) * 100) >> 16);
    u8g2_DrawStr(&u8g2, 2, 22, tempStr);

    snprintf(tempStr, sizeof(tempStr), "Sw %d  Pot %d  %d", switchState, pot, filtMode);
    u8g2_DrawStr(&u8g2, 2, 34, tempStr);

    snprintf(tempStr, sizeof(tempStr), "SWR %d.%02d", swr >> 16, ((swr & 0xFFFF) *100) >> 16);
    u8g2_DrawStr(&u8g2, 2, 46, tempStr);
    
    //u8g2_DrawBox(&u8g2, 100, 0, 2, 64);
    //u8g2_DrawBox(&u8g2, 0, 60, 128, 2);

#else
    // Clear the display
    // invert if SWR is > 2.0
    if(swr >= WARN_SWR_LEVEL)
    {
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawBox(&u8g2, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        u8g2_SetDrawColor(&u8g2, 0);
    }
    else
    {
        u8g2_ClearBuffer(&u8g2);
        u8g2_SetDrawColor(&u8g2, 1);
    }
    
    // Write the measurements on the top line
    int top = SCREEN_TOP;
    switch(switchState)
    {
        case SWITCH_200:
        case SWITCH_2000:
            // Add value of meter item in upper right
            PrintNumber(tempStr, sizeof(tempStr), "Fwd:", FwdFilt);            
            u8g2_DrawStr(&u8g2, SCREEN_WIDTH / 2, top + FONT_HEIGHT, tempStr);

            top = DrawElement("Ref:", RefFilt, 0, WATT2INTPOWER(100), top);
    
            top += MARGIN * 2;
            DrawElement("SWR:",  swr, SWR2INTSWR(1), SWR2INTSWR(5), top);
            break;
        case SWITCH_SWR:
            // Add value of meter item in upper right
            PrintNumber(tempStr, sizeof(tempStr), "SWR:", swr);
            u8g2_DrawStr(&u8g2, SCREEN_WIDTH / 2, top + FONT_HEIGHT, tempStr);

            top = DrawElement("Ref:",  RefFilt, 0, WATT2INTPOWER(100), top);
    
            top += MARGIN * 2;
            DrawElement("Fwd:", FwdFilt, 0, fwdHighRange ? WATT2INTPOWER(1500) : WATT2INTPOWER(150), top);
            break;
        default:
            // do nothing, we don't have a valid switch state
            break;
    }
    // Add filter mode on right side
    char* filtStr;
    switch(filtMode)
    {
        case FILT_SLOW:     filtStr = "S";  break;
        case FILT_MED:      filtStr = "M";  break;
        case FILT_FAST:     filtStr = "F";  break;
        case FILT_PEAKFAST: filtStr = "PF"; break;
        case FILT_PEAKMED:  filtStr = "PM"; break;
        case FILT_PEAKSLOW: filtStr = "PS"; break;
        default:            filtStr = "ER"; break;
    }
    u8g2_DrawStr(&u8g2, 100, top + FONT_HEIGHT, filtStr);
    #endif
    // Send contents to screen
    u8g2_SendBuffer(&u8g2);    

}



/* [] END OF FILE */
