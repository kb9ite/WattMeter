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
#include "u8g2Support.h"

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch(msg)
    {
        case U8X8_MSG_DELAY_10MICRO:		// delay arg_int * 10 micro seconds
            // Spin waiting sucks, but I don't this is used much    
            CyDelayUs(10);
            break;
        case U8X8_MSG_DELAY_MILLI:			// delay arg_int * 1 milli second
            // Spin waiting sucks, but I don't this is used much
            CyDelay(1);
            break;
        case U8X8_MSG_GPIO_DC:				// DC (data/cmd, A0, register select) pin: Output level in arg_int
            if(arg_int)
            {
                LCD_Reg_Control |= 2;
            }
            else
            {
                LCD_Reg_Control &= 0xFD;
            }
            break;

        case U8X8_MSG_GPIO_RESET:			// Reset pin: Output level in arg_int
            if(arg_int)
            {
                LCD_Reg_Control |= 1;
            }
            else
            {
                LCD_Reg_Control &= 0xFE;
            }
          break;
            
        case U8X8_MSG_GPIO_AND_DELAY_INIT:	// called once during init phase of u8g2/u8x8
            // Not needed, IO is initialized elsewhere.     
            break;
        case U8X8_MSG_DELAY_100NANO:		// delay arg_int * 100 nano seconds
            // There is not a CyDelay function that supports this resolution, but this case
            // doesn't appear to be used anywhere.
            break;
        case U8X8_MSG_DELAY_NANO:			// delay arg_int * 1 nano second
            // There is not a CyDelay function that supports this resolution, but this case
            // doesn't appear to be used anywhere.
            break;    
/*        case U8X8_MSG_DELAY_I2C:				// arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
          break;							// arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
        case U8X8_MSG_GPIO_D0:				// D0 or SPI clock pin: Output level in arg_int
        //case U8X8_MSG_GPIO_SPI_CLOCK:
          break;
        case U8X8_MSG_GPIO_D1:				// D1 or SPI data pin: Output level in arg_int
        //case U8X8_MSG_GPIO_SPI_DATA:
          break;
        case U8X8_MSG_GPIO_D2:				// D2 pin: Output level in arg_int
          break;
        case U8X8_MSG_GPIO_D3:				// D3 pin: Output level in arg_int
          break;
        case U8X8_MSG_GPIO_D4:				// D4 pin: Output level in arg_int
          break;
        case U8X8_MSG_GPIO_D5:				// D5 pin: Output level in arg_int
          break;
        case U8X8_MSG_GPIO_D6:				// D6 pin: Output level in arg_int
          break;
        case U8X8_MSG_GPIO_D7:				// D7 pin: Output level in arg_int
          break;
        case U8X8_MSG_GPIO_E:				// E/WR pin: Output level in arg_int
          break;
        case U8X8_MSG_GPIO_CS:				// CS (chip select) pin: Output level in arg_int
          break;
        case U8X8_MSG_GPIO_CS1:				// CS1 (chip select) pin: Output level in arg_int
          break;
        case U8X8_MSG_GPIO_CS2:				// CS2 (chip select) pin: Output level in arg_int
          break;
        case U8X8_MSG_GPIO_I2C_CLOCK:		// arg_int=0: Output low at I2C clock pin
          break;							// arg_int=1: Input dir with pullup high for I2C clock pin
        case U8X8_MSG_GPIO_I2C_DATA:			// arg_int=0: Output low at I2C data pin
          break;							// arg_int=1: Input dir with pullup high for I2C data pin
            */
        case U8X8_MSG_GPIO_MENU_SELECT:
          u8x8_SetGPIOResult(u8x8, /* get menu select pin state */ 0);
          break;
        case U8X8_MSG_GPIO_MENU_NEXT:
          u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
          break;
        case U8X8_MSG_GPIO_MENU_PREV:
          u8x8_SetGPIOResult(u8x8, /* get menu prev pin state */ 0);
          break;
        case U8X8_MSG_GPIO_MENU_HOME:
          u8x8_SetGPIOResult(u8x8, /* get menu home pin state */ 0);
          break;
        default:
          u8x8_SetGPIOResult(u8x8, 1);			// default return value
          break;
    }
    return 1;
}
uint8 sendZeros = 0;
uint8_t zeros[128];

uint8_t u8x8_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) 
{
    switch(msg) 
    {
        case U8X8_MSG_BYTE_SEND:
            SPI_SpiUartPutArray(arg_ptr, arg_int);
            CY_NOP;
            CY_NOP;
            while(SPI_SpiIsBusBusy() != 0)
            {
                //Spin wait for transfer to be complete
            }
            for(int i = 0; i < 10; i++)
            {
                CY_NOP;
            }
            break;
        case U8X8_MSG_BYTE_INIT:
            break;
        case U8X8_MSG_BYTE_SET_DC:
            u8x8_gpio_SetDC(u8x8, arg_int);
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:      
            break;
        default:
            return 0;
    }  
    return 1;
}

/* [] END OF FILE */
