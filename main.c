

/* Microcontroller MIPs (FCY) */
#define SYS_FREQ        40000000L
#define FCY             SYS_FREQ/4
#define _XTAL_FREQ      SYS_FREQ


// CONFIG1L
#pragma config WDTEN = OFF      // Watchdog Timer (Disabled - Controlled by SWDTEN bit)
#pragma config PLLDIV = 1       // 96MHz PLL Prescaler Selection (PLLSEL=0) (No prescale (4 MHz oscillator input drives PLL directly))
#pragma config CFGPLLEN = OFF//ON    // PLL Enable Configuration Bit (PLL Enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset (Enabled)
#pragma config XINST = OFF      // Extended Instruction Set (Disabled)

// CONFIG1H
#pragma config CP0 = OFF        // Code Protect (Program memory is not code-protected)

// CONFIG2L
#pragma config OSC = EC
#pragma config SOSCSEL = DIG    // T1OSC/SOSC Power Selection Bits (Digital (SCLKI) mode selected)
#pragma config CLKOEC = ON      // EC Clock Out Enable Bit  (CLKO output enabled on the RA6 pin)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor (Disabled)
#pragma config IESO = OFF       // Internal External Oscillator Switch Over Mode (Disabled)

// CONFIG2H
#pragma config WDTPS = 32768    // Watchdog Postscaler (1:32768)

// CONFIG3L
#pragma config DSWDTOSC = INTOSCREF// DSWDT Clock Select (DSWDT uses INTRC)
#pragma config RTCOSC = INTOSCREF// RTCC Clock Select (RTCC uses INTRC)
#pragma config DSBOREN = OFF    // Deep Sleep BOR (Disabled)
#pragma config DSWDTEN = OFF    // Deep Sleep Watchdog Timer (Disabled)
#pragma config DSWDTPS = 2      // Deep Sleep Watchdog Postscaler (1:2 (2.1 ms))

// CONFIG3H
#pragma config IOL1WAY = OFF    // IOLOCK One-Way Set Enable bit (The IOLOCK bit (PPSCON<0>) can be set and cleared as needed)
#pragma config ADCSEL = BIT10   // ADC 10 or 12 Bit Select (10 - Bit ADC Enabled)
#pragma config PLLSEL = PLL4X   // PLL Selection Bit (Selects 4x PLL)
#pragma config MSSP7B_EN = MSK7 // MSSP address masking (7 Bit address masking mode)

// CONFIG4L
#pragma config WPFP = PAGE_127  // Write/Erase Protect Page Start/End Location (Write Protect Program Flash Page 127)
#pragma config WPCFG = OFF      // Write/Erase Protect Configuration Region  (Configuration Words page not erase/write-protected)

// CONFIG4H
#pragma config WPDIS = OFF      // Write Protect Disable bit (WPFP<6:0>/WPEND region ignored)
#pragma config WPEND = PAGE_WPFP// Write/Erase Protect Region Select bit (valid when WPDIS = 0) (Pages WPFP<6:0> through Configuration Words erase/write protected)


/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>        /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>       /* HiTech General Include File */
#elif defined(__18CXX)
    #include <p18cxxx.h>   /* C18 General Include File */
#endif

#if defined(__XC) || defined(HI_TECH_C)

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */

#endif




unsigned int tmr0value = 0x0000;
unsigned int tmr0counter = 0x0000;

void sound(unsigned int value, unsigned int duration)
{
	tmr0value = value;
	tmr0counter = duration;
	
	if (value == 0x0000 || duration == 0x0000) TMR0ON = 0;
	else TMR0ON = 1;
};


unsigned char keyarray[256];
unsigned char keybuffer = 0x00;
unsigned char keycurr = 0x00;
unsigned char keylast = 0x00;
unsigned char keyextended = 0x00;
unsigned char keyrelease = 0x00;
unsigned char keyshift = 0x00;
unsigned char keycapslock = 0x00;
unsigned char keycounter = 0x00;
unsigned char keyreadpos = 0x00;
unsigned char keywritepos = 0x00;


/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/

/* High-priority service */

#if defined(__XC) || defined(HI_TECH_C)
void __interrupt(high_priority) high_isr(void)
#elif defined (__18CXX)
#pragma code high_isr=0x08
#pragma interrupt high_isr
void high_isr(void)
#else
#error "Invalid compiler selection for implemented ISR routines"
#endif

{

      /* This code stub shows general interrupt handling.  Note that these
      conditional statements are not handled within 3 seperate if blocks.
      Do not use a seperate if block for each interrupt flag to avoid run
      time errors. */

#if 0
    
      /* TODO Add High Priority interrupt routine code here. */

      /* Determine which flag generated the interrupt */
      if(<Interrupt Flag 1>)
      {
          <Interrupt Flag 1=0>; /* Clear Interrupt Flag 1 */
      }
      else if (<Interrupt Flag 2>)
      {
          <Interrupt Flag 2=0>; /* Clear Interrupt Flag 2 */
      }
      else
      {
          /* Unhandled interrupts */
      }

#endif
	
	if (RBIF == 1)
	{
		RBIF = 0; // clear interrupts for RB

		keycurr = PORTB;

		if ((keylast & 0x40) == 0x40 && (keycurr & 0x40) == 0x00)
		{		
			if (keycounter < 0x09)
			{
				keybuffer = keybuffer >> 1;
				keybuffer = keybuffer | (keycurr & 0x80);
			}

			keycounter++;

			if (keycounter == 0x0B)
			{
				keycounter = 0x00;

				keyarray[keywritepos] = keybuffer;

				keywritepos++;
			}
		}

		keylast = keycurr;
	}
	else if (TMR0IF == 1)
	{
		TMR0IF = 0; // clear interrupts for TMR0
		
		TMR0 = tmr0value; // reset TMR0
		
		if (RA5 == 0) LATA = LATA | 0xF0;
		else LATA = LATA & 0x0F; // toggle RA5
		
		tmr0counter--;
		
		if (tmr0counter == 0x0000) TMR0ON = 0;
	}
	 
	

}

/* Low-priority interrupt routine */
#if defined(__XC) || defined(HI_TECH_C)
void __interrupt(low_priority) low_isr(void)
#elif defined (__18CXX)
#pragma code low_isr=0x18
#pragma interruptlow low_isr
void low_isr(void)
#else
#error "Invalid compiler selection for implemented ISR routines"
#endif
{

      /* This code stub shows general interrupt handling.  Note that these
      conditional statements are not handled within 3 seperate if blocks.
      Do not use a seperate if block for each interrupt flag to avoid run
      time errors. */

#if 0

      /* TODO Add Low Priority interrupt routine code here. */

      /* Determine which flag generated the interrupt */
      if(<Interrupt Flag 1>)
      {
          <Interrupt Flag 1=0>; /* Clear Interrupt Flag 1 */
      }
      else if (<Interrupt Flag 2>)
      {
          <Interrupt Flag 2=0>; /* Clear Interrupt Flag 2 */
      }
      else
      {
          /* Unhandled interrupts */
      }

#endif

}




/******************************************************************************/
/* User Global Variable Declaration                                           */
/******************************************************************************/

/* i.e. uint8_t <variable_name>; */


#include "ExtendedMemory.c"



const unsigned char bitmap[1536] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 32
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x0F,0x00,0x3F,0xC0,0x3F,0xC0,0x3F,0xC0,
	0x0F,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,
	0x0C,0x30,0x30,0xC0,0x3C,0xF0,0x3C,0xF0,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x3C,0xF0,0xFF,0xFC,0x3C,0xF0,0x3C,0xF0,
	0x3C,0xF0,0xFF,0xFC,0x3C,0xF0,0x00,0x00,
	0x03,0x00,0x3F,0xFC,0xF3,0x00,0x3F,0xF0,
	0x03,0x3C,0xFF,0xF0,0x03,0x00,0x00,0x00,
	0x30,0x0C,0xCC,0x3C,0x30,0xF0,0x03,0xC0,
	0x0F,0x30,0x3C,0xCC,0xF0,0x30,0x00,0x00,
	0x3F,0x00,0xF3,0xC0,0x3F,0x00,0xF3,0xCC,
	0xF0,0xFC,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0x0F,0x00,0x0F,0x00,0x03,0x00,0x0C,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x03,0xF0,0x0F,0x00,0x3C,0x00,0x3C,0x00,
	0x3C,0x00,0x0F,0x00,0x03,0xF0,0x00,0x00,
	0x3F,0x00,0x03,0xC0,0x00,0xF0,0x00,0xF0,
	0x00,0xF0,0x03,0xC0,0x3F,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x3C,0xF0,0x0F,0xC0,
	0x3C,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x03,0x00,0x03,0x00,
	0x3F,0xF0,0x03,0x00,0x03,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,
	0x0F,0x00,0x03,0x00,0x0C,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x3F,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x0F,0x00,0x0F,0x00,0x00,0x00,
	0x00,0x30,0x00,0xF0,0x03,0xC0,0x0F,0x00,
	0x3C,0x00,0xF0,0x00,0xC0,0x00,0x00,0x00,
	0x3F,0xF0,0xF0,0x3C,0xF0,0xFC,0xF3,0x3C,
	0xFC,0x3C,0xF0,0x3C,0x3F,0xF0,0x00,0x00, // 65
	0xFF,0xC0,0x0F,0xC0,0x0F,0xC0,0x0F,0xC0,
	0x0F,0xC0,0x0F,0xC0,0xFF,0xFC,0x00,0x00,
	0x3F,0xF0,0xF0,0x3C,0x00,0xF0,0x0F,0xC0,
	0x3C,0x00,0xF0,0x00,0xFF,0xFC,0x00,0x00,
	0x3F,0xF0,0xF0,0x3C,0x00,0x3C,0x0F,0xF0,
	0x00,0x3C,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0x03,0xFC,0x0F,0x3C,0x3C,0x3C,0xF0,0x3C,
	0xFF,0xFC,0x00,0x3C,0x00,0x3C,0x00,0x00,
	0xFF,0xFC,0xF0,0x00,0xFF,0xF0,0x00,0x3C,
	0x00,0x3C,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0x3F,0xF0,0xF0,0x3C,0xF0,0x00,0xFF,0xF0,
	0xF0,0x3C,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0xFF,0xFC,0xF0,0x3C,0x00,0xF0,0x03,0xC0,
	0x0F,0x00,0x3C,0x00,0xF0,0x00,0x00,0x00,
	0x3F,0xF0,0xF0,0x3C,0xF0,0x3C,0x3F,0xF0,
	0xF0,0x3C,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0x3F,0xF0,0xF0,0x3C,0xF0,0x3C,0x3F,0xFC,
	0x00,0x3C,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,
	0x0F,0x00,0x0F,0x00,0x3C,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x03,0xF0,0x0F,0xC0,
	0x3F,0x00,0x0F,0xC0,0x03,0xF0,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xF0,
	0x00,0x00,0x3F,0xF0,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x3F,0x00,0x0F,0xC0,
	0x03,0xF0,0x0F,0xC0,0x3F,0x00,0x00,0x00,
	0x3F,0xF0,0xF0,0x3C,0x00,0x3C,0x0F,0xF0,
	0x0F,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,
	0x3F,0xF0,0xF0,0x3C,0xF3,0x3C,0xF3,0x3C,
	0xF3,0xF0,0xF0,0x00,0x3F,0xF0,0x00,0x00,
	0x0F,0xC0,0x3C,0xF0,0xF0,0x3C,0xF0,0x3C,
	0xFF,0xFC,0xF0,0x3C,0xF0,0x3C,0x00,0x00,
	0xFF,0xF0,0xF0,0x3C,0xF0,0x3C,0xFF,0xF0,
	0xF0,0x3C,0xF0,0x3C,0xFF,0xF0,0x00,0x00,
	0x3F,0xF0,0xF0,0x3C,0xF0,0x00,0xF0,0x00,
	0xF0,0x00,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0xFF,0xC0,0xF0,0xF0,0xF0,0x3C,0xF0,0x3C,
	0xF0,0x3C,0xF0,0xF0,0xFF,0xC0,0x00,0x00,
	0xFF,0xFC,0xF0,0x00,0xF0,0x00,0xFF,0xC0,
	0xF0,0x00,0xF0,0x00,0xFF,0xFC,0x00,0x00,
	0xFF,0xFC,0xF0,0x00,0xF0,0x00,0xFF,0xC0,
	0xF0,0x00,0xF0,0x00,0xF0,0x00,0x00,0x00,
	0x3F,0xFC,0xF0,0x00,0xF0,0x00,0xF3,0xFC,
	0xF0,0x3C,0xF0,0x3C,0x3F,0xFC,0x00,0x00,
	0xF0,0x3C,0xF0,0x3C,0xF0,0x3C,0xFF,0xFC,
	0xF0,0x3C,0xF0,0x3C,0xF0,0x3C,0x00,0x00,
	0xFF,0xFC,0x0F,0xC0,0x0F,0xC0,0x0F,0xC0,
	0x0F,0xC0,0x0F,0xC0,0xFF,0xFC,0x00,0x00,
	0x03,0xFC,0x00,0x3C,0x00,0x3C,0x00,0x3C,
	0xF0,0x3C,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0xF0,0x3C,0xF0,0xF0,0xF3,0xC0,0xFF,0x00,
	0xF3,0xC0,0xF0,0xF0,0xF0,0x3C,0x00,0x00,
	0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,
	0xF0,0x00,0xF0,0x00,0xFF,0xFC,0x00,0x00,
	0xF0,0x3C,0xFC,0xFC,0xFF,0xFC,0xF3,0x3C,
	0xF0,0x3C,0xF0,0x3C,0xF0,0x3C,0x00,0x00,
	0xF0,0x3C,0xFC,0x3C,0xFF,0x3C,0xF3,0xFC,
	0xF0,0xFC,0xF0,0x3C,0xF0,0x3C,0x00,0x00,
	0x3F,0xF0,0xF0,0x3C,0xF0,0x3C,0xF0,0x3C,
	0xF0,0x3C,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0xFF,0xF0,0xF0,0x3C,0xF0,0x3C,0xFF,0xF0,
	0xF0,0x00,0xF0,0x00,0xF0,0x00,0x00,0x00,
	0x3F,0xF0,0xF0,0x3C,0xF0,0x3C,0xF0,0x3C,
	0xF3,0xFC,0xF0,0xF0,0x3F,0xCC,0x00,0x00,
	0xFF,0xF0,0xF0,0x3C,0xF0,0x3C,0xFF,0xF0,
	0xF3,0xC0,0xF0,0xF0,0xF0,0x3C,0x00,0x00,
	0x3F,0xF0,0xF0,0x3C,0xF0,0x00,0x3F,0xF0,
	0x00,0x3C,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0xFF,0xFC,0x0F,0xC0,0x0F,0xC0,0x0F,0xC0,
	0x0F,0xC0,0x0F,0xC0,0x0F,0xC0,0x00,0x00,
	0xF0,0x3C,0xF0,0x3C,0xF0,0x3C,0xF0,0x3C,
	0xF0,0x3C,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0xF0,0x3C,0xF0,0x3C,0xF0,0x3C,0xF0,0x3C,
	0x3C,0xF0,0x0F,0xC0,0x03,0x00,0x00,0x00,
	0xF0,0x3C,0xF0,0x3C,0xF3,0x3C,0xFF,0xFC,
	0xFC,0xFC,0xF0,0x3C,0xF0,0x3C,0x00,0x00,
	0xF0,0x3C,0xFC,0xFC,0x3F,0xF0,0x0F,0xC0,
	0x3F,0xF0,0xFC,0xFC,0xF0,0x3C,0x00,0x00,
	0xF0,0x3C,0xF0,0x3C,0xF0,0x3C,0x3F,0xF0,
	0x0F,0xC0,0x0F,0xC0,0x0F,0xC0,0x00,0x00,
	0xFF,0xFC,0x00,0xFC,0x03,0xF0,0x0F,0xC0,
	0x3F,0x00,0xFC,0x00,0xFF,0xFC,0x00,0x00,
	0x3F,0xF0,0x3C,0x00,0x3C,0x00,0x3C,0x00,
	0x3C,0x00,0x3C,0x00,0x3F,0xF0,0x00,0x00,
	0x30,0x00,0x3C,0x00,0x0F,0x00,0x03,0xC0,
	0x00,0xF0,0x00,0x3C,0x00,0x0C,0x00,0x00,
	0x3F,0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,
	0x00,0xF0,0x00,0xF0,0x3F,0xF0,0x00,0x00,
	0x03,0x00,0x0F,0xC0,0x3C,0xF0,0xF0,0x3C,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFF,0xFC,0x00,0x00,
	0x0C,0x00,0x03,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x3F,0xF0,0x00,0x3C,
	0x3F,0xFC,0xF0,0x3C,0x3F,0xFC,0x00,0x00,
	0xF0,0x00,0xF0,0x00,0xFF,0xF0,0xF0,0x3C,
	0xF0,0x3C,0xF0,0x3C,0xFF,0xF0,0x00,0x00,
	0x00,0x00,0x00,0x00,0x3F,0xF0,0xF0,0x3C,
	0xF0,0x00,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0x00,0x3C,0x00,0x3C,0x3F,0xFC,0xF0,0x3C,
	0xF0,0x3C,0xF0,0x3C,0x3F,0xFC,0x00,0x00,
	0x00,0x00,0x00,0x00,0x3F,0xF0,0xF0,0x3C,
	0xFF,0xFC,0xF0,0x00,0x3F,0xF0,0x00,0x00,
	0x0F,0xF0,0x0F,0x00,0xFF,0xFC,0x0F,0x00,
	0x0F,0x00,0x0F,0x00,0x0F,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x3F,0xFC,0xF0,0x3C,
	0x3F,0xFC,0x00,0x3C,0x3F,0xF0,0x00,0x00,
	0xF0,0x00,0xF0,0x00,0xFF,0xF0,0xF0,0x3C,
	0xF0,0x3C,0xF0,0x3C,0xF0,0x3C,0x00,0x00,
	0x03,0xC0,0x00,0x00,0x3F,0xC0,0x03,0xC0,
	0x03,0xC0,0x03,0xC0,0xFF,0xFC,0x00,0x00,
	0x00,0x3C,0x00,0x00,0x00,0x3C,0x00,0x3C,
	0x00,0x3C,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0xF0,0x00,0xF0,0x00,0xF0,0x3C,0xF0,0xF0,
	0xFF,0xC0,0xF0,0xF0,0xF0,0x3C,0x00,0x00,
	0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,
	0xF0,0x00,0x3F,0x00,0x03,0xFC,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFF,0xF0,0xF3,0x3C,
	0xF3,0x3C,0xF3,0x3C,0xF3,0x3C,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFF,0xF0,0xF0,0x3C,
	0xF0,0x3C,0xF0,0x3C,0xF0,0x3C,0x00,0x00,
	0x00,0x00,0x00,0x00,0x3F,0xF0,0xF0,0x3C,
	0xF0,0x3C,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFF,0xF0,0xF0,0x3C,
	0xFF,0xF0,0xF0,0x00,0xF0,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x3F,0xFC,0xF0,0x3C,
	0x3F,0xFC,0x00,0x3C,0x00,0x3C,0x00,0x00,
	0x00,0x00,0x00,0x00,0xF3,0xF0,0xFC,0x3C,
	0xF0,0x00,0xF0,0x00,0xF0,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x3F,0xFC,0xF0,0x00,
	0xFF,0xFC,0x00,0x3C,0xFF,0xF0,0x00,0x00,
	0x0F,0x00,0x0F,0x00,0xFF,0xFC,0x0F,0x00,
	0x0F,0x00,0x0F,0x00,0x0F,0xFC,0x00,0x00,
	0x00,0x00,0x00,0x00,0xF0,0x3C,0xF0,0x3C,
	0xF0,0x3C,0xF0,0x3C,0x3F,0xF0,0x00,0x00,
	0x00,0x00,0x00,0x00,0xF0,0x3C,0xF0,0x3C,
	0x3C,0xF0,0x0F,0xC0,0x03,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xF0,0x3C,0xF3,0x3C,
	0xF3,0x3C,0xFF,0xFC,0x3C,0xF0,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFC,0xFC,0x3F,0xF0,
	0x0F,0xC0,0x3F,0xF0,0xFC,0xFC,0x00,0x00,
	0x00,0x00,0x00,0x00,0xF0,0x3C,0xF0,0x3C,
	0x3C,0xF0,0x0F,0xC0,0xFF,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFF,0xFC,0x03,0xF0,
	0x0F,0xC0,0x3F,0x00,0xFF,0xFC,0x00,0x00,
	0x03,0xF0,0x0F,0x00,0x0F,0x00,0x3F,0x00,
	0x0F,0x00,0x0F,0x00,0x03,0xF0,0x00,0x00,
	0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,
	0x0F,0x00,0x0F,0x00,0x0F,0x00,0x00,0x00,
	0x3F,0x00,0x03,0xC0,0x03,0xC0,0x03,0xF0,
	0x03,0xC0,0x03,0xC0,0x3F,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0x3C,
	0xF3,0x3C,0xF3,0xF0,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

const unsigned char conversion[256] = 
{
  	0x00,0x16,0x0C,0x0E,0x1E,0x1C,0x1D,0x15,
	0x00,0x18,0x07,0x0F,0x1F,0x09,0x60,0x00,
	0x00,0x00,0x00,0x00,0x00,0x71,0x31,0x00,
	0x00,0x00,0x7A,0x73,0x61,0x77,0x32,0x00,
	0x00,0x63,0x78,0x64,0x65,0x34,0x33,0x00,
	0x00,0x20,0x76,0x66,0x74,0x72,0x35,0x00,
	0x00,0x6E,0x62,0x68,0x67,0x79,0x36,0x00,
	0x00,0x00,0x6D,0x6A,0x75,0x37,0x38,0x00,
	0x00,0x2C,0x6B,0x69,0x6F,0x30,0x39,0x00,
	0x00,0x2E,0x2F,0x6C,0x3B,0x70,0x2D,0x00,
	0x00,0x00,0x27,0x00,0x5B,0x3D,0x00,0x00,
	0x00,0x00,0x0D,0x5D,0x00,0x5C,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,
	0x00,0x31,0x00,0x34,0x37,0x00,0x00,0x00,
	0x30,0x2E,0x32,0x35,0x36,0x38,0x1B,0x00,
	0x19,0x2B,0x33,0x2D,0x2A,0x39,0x00,0x00,

	0x00,0x16,0x0C,0x0E,0x1E,0x1C,0x1D,0x15,
	0x00,0x18,0x07,0x0F,0x1F,0x09,0x7E,0x00,
	0x00,0x00,0x00,0x00,0x00,0x51,0x21,0x00,
	0x00,0x00,0x5A,0x53,0x41,0x57,0x40,0x00,
	0x00,0x43,0x58,0x44,0x45,0x24,0x23,0x00,
	0x00,0x20,0x56,0x46,0x54,0x52,0x25,0x00,
	0x00,0x4E,0x42,0x48,0x47,0x59,0x5E,0x00,
	0x00,0x00,0x4D,0x4A,0x55,0x26,0x2A,0x00,
	0x00,0x3C,0x4B,0x49,0x4F,0x29,0x28,0x00,
	0x00,0x3E,0x3F,0x4C,0x3A,0x50,0x5F,0x00,
	0x00,0x00,0x22,0x00,0x7B,0x2B,0x00,0x00,
	0x00,0x00,0x0D,0x7D,0x00,0x7C,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,
	0x00,0x03,0x00,0x13,0x02,0x00,0x00,0x00,
	0x1A,0x7F,0x12,0x35,0x14,0x11,0x1B,0x00,
	0x19,0x2B,0x04,0x2D,0x2A,0x01,0x00,0x00
};

const unsigned char random[224] = 
{
	0, 4, 3, 6, 2, 5, 1,
	4, 1, 3, 5, 2, 0, 6,
	6, 5, 2, 0, 4, 1, 3,
	5, 4, 6, 0, 3, 1, 2,
	
	6, 0, 1, 5, 3, 4, 2,
	2, 4, 0, 5, 1, 6, 3,
	3, 1, 6, 2, 4, 0, 5,
	4, 5, 1, 6, 0, 2, 3,
	
	6, 5, 0, 1, 4, 3, 2,
	3, 5, 2, 6, 1, 0, 4,
	1, 3, 6, 5, 4, 2, 0,
	2, 4, 6, 0, 1, 3, 5,
	
	5, 2, 6, 0, 4, 1, 3,
	5, 0, 2, 4, 6, 1, 3,
	1, 4, 2, 6, 0, 3, 5,
	6, 0, 3, 2, 5, 4, 1,
	
	
	2, 6, 3, 5, 4, 0, 1,
	1, 3, 2, 6, 5, 0, 4,
	6, 4, 0, 3, 5, 2, 1,
	2, 5, 3, 4, 6, 0, 1,
	
	4, 5, 0, 6, 2, 1, 3,
	2, 1, 3, 0, 4, 5, 6,
	2, 0, 5, 4, 6, 1, 3,
	6, 2, 3, 0, 5, 4, 1,
	
	0, 6, 5, 3, 1, 4, 2,
	4, 5, 1, 2, 0, 6, 3,
	4, 1, 6, 3, 0, 2, 5,
	6, 0, 2, 5, 4, 3, 1,
	
	1, 3, 5, 2, 6, 0, 4,
	3, 1, 2, 6, 4, 0, 5,
	4, 2, 0, 1, 6, 3, 5,
	4, 3, 0, 5, 1, 6, 2,
};



// tells if the initialization process worked or not
unsigned char sdcard_ready = 0x00;

// blocks for SDcard
unsigned char sdcard_block_high = 0x00;
unsigned char sdcard_block_mid = 0x00;
unsigned char sdcard_block_low = 0x00;

unsigned char sdcard_block_buffer[512] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


// SDcard commands below
// This was used for the Arduino, but has been modified to work here.
void sdcard_enable(void)
{
	RA2 = 0; // CS is low
}

void sdcard_disable(void)
{
	RA2 = 1; // CS is high
}

void sdcard_longdelay(void)
{
	unsigned int tally = 0x0000;

	while (tally < 0x0400) // arbitrary amount of time to delay, should be around 10ms
	{
		tally = tally + 1;
	}
}

void sdcard_toggle(void)
{
	RA0 = 1; // CLK is high
	RA0 = 0; // CLK is low
}

void sdcard_pump(void)
{
	RA2 = 1; // CS is high, must disable the device
	RA1 = 1; // MOSI is high, AND leave mosi high!!!

	sdcard_longdelay();

	for (unsigned char i=0; i<80; i++)
	{
		sdcard_toggle();
	}
}


void sdcard_sendbyte(unsigned char value)
{
	unsigned char temp_value = value;

	for (unsigned char i=0; i<8; i++)
	{
		if (temp_value >= 0x80)
		{
			RA1 = 1; // MOSI is high
		}
		else
		{
			RA1 = 0; // MOSI is low
		}

		temp_value = temp_value << 1;

		sdcard_toggle();
	}
};


unsigned char sdcard_receivebyte(void)
{
	unsigned char temp_value = 0x00;

	for (unsigned char i=0; i<8; i++)
	{
		temp_value = temp_value << 1;

		if (RB4) // if MISO is high...
		{
			temp_value += 0x01;
		}

		sdcard_toggle();
	}
	
	return temp_value;
};


unsigned char sdcard_waitresult(void)
{
	unsigned char temp_value = 0xFF;

	for (unsigned int i=0; i<2048; i++) // arbitrary wait time
	{
		temp_value = sdcard_receivebyte();

		if (temp_value != 0xFF)
		{
			return temp_value;
		}
	}

	return 0xFF;
}

unsigned char sdcard_initialize(void)
{
	unsigned char temp_value = 0x00;

	RA2 = 1; // CS is high
	RA1 = 1; // MOSI is high
	RA0 = 0; // CLK is low

	sdcard_disable();
	sdcard_pump();
	sdcard_longdelay();
	sdcard_enable();
	sdcard_sendbyte(0x40); // CMD0 = 0x40 + 0x00 (0 in hex)
	sdcard_sendbyte(0x00);
	sdcard_sendbyte(0x00);
	sdcard_sendbyte(0x00);
	sdcard_sendbyte(0x00);
	sdcard_sendbyte(0x95); // CRC for CMD0
	temp_value = sdcard_waitresult(); // command response
	if (temp_value == 0xFF) { return 0; }
	sdcard_disable();
	if (temp_value != 0x01) { return 0; } // expecting 0x01
	sdcard_longdelay();
	sdcard_pump();
	sdcard_enable();
	sdcard_sendbyte(0x48); // CMD8 = 0x40 + 0x08 (8 in hex)
	sdcard_sendbyte(0x00); // CMD8 needs 0x000001AA argument
	sdcard_sendbyte(0x00);
	sdcard_sendbyte(0x01);
	sdcard_sendbyte(0xAA); 
	sdcard_sendbyte(0x87); // CRC for CMD8
	temp_value = sdcard_waitresult(); // command response
	if (temp_value == 0xFF) { return 0; }
	sdcard_disable();
	if (temp_value != 0x01) { return 0; } // expecting 0x01
	sdcard_enable();
	temp_value = sdcard_receivebyte(); // 32-bit return value, ignore
	temp_value = sdcard_receivebyte();
	temp_value = sdcard_receivebyte();
	temp_value = sdcard_receivebyte();
	sdcard_disable();
	do {
		sdcard_pump();
		sdcard_longdelay();
		sdcard_enable();
		sdcard_sendbyte(0x77); // CMD55 = 0x40 + 0x37 (55 in hex)
		sdcard_sendbyte(0x00);
		sdcard_sendbyte(0x00);
		sdcard_sendbyte(0x00);
		sdcard_sendbyte(0x00);
		sdcard_sendbyte(0x01); // CRC (general)
		temp_value = sdcard_waitresult(); // command response
		if (temp_value == 0xFF) { return 0; }
		sdcard_disable();
		if (temp_value != 0x01) { break; } // expecting 0x01, but if not it might already be 'initialized'?
		sdcard_pump();
		sdcard_longdelay();
		sdcard_enable();
		sdcard_sendbyte(0x69); // CMD41 = 0x40 + 0x29 (41 in hex)
		sdcard_sendbyte(0x40); // needed for CMD41?
		sdcard_sendbyte(0x00);
		sdcard_sendbyte(0x00);
		sdcard_sendbyte(0x00);
		sdcard_sendbyte(0x01); // CRC (general)
		temp_value = sdcard_waitresult(); // command response
		if (temp_value == 0xFF) { return 0; }
		sdcard_disable();
		if (temp_value != 0x00 && temp_value != 0x01) { return 0; } // expecting 0x00, if 0x01 try again
		sdcard_longdelay();
	} while (temp_value == 0x01);

	sdcard_ready = 0x01;

	return 1;
}

unsigned char sdcard_readinit(unsigned char high, unsigned char mid, unsigned char low)
{
	unsigned char temp_value = 0x00;

	sdcard_disable();
	sdcard_pump();
	sdcard_longdelay();
	sdcard_enable();
	sdcard_sendbyte(0x51); // CMD17 = 0x40 + 0x11 (17 in hex)
	sdcard_sendbyte(high);
	sdcard_sendbyte(mid);
	sdcard_sendbyte((low&0xFE)); // only blocks of 512 bytes
	sdcard_sendbyte(0x00);
	sdcard_sendbyte(0x01); // CRC (general)
	temp_value = sdcard_waitresult(); // command response
	if (temp_value == 0xFF) { return 0; }
	else if (temp_value != 0x00) { return 0; } // expecting 0x00
	temp_value = sdcard_waitresult(); // data packet starts with 0xFE
	if (temp_value == 0xFF) { return 0; }
	else if (temp_value != 0xFE) { return 0; }
	
	return 1;
}

unsigned char sdcard_readfinal(void)
{
	unsigned char temp_value = 0x00;

	temp_value = sdcard_receivebyte(); // data packet ends with 0x55 then 0xAA
	temp_value = sdcard_receivebyte(); // ignore here
	sdcard_disable();

	return 1;
}

unsigned char sdcard_readblock()
{
	unsigned char temp_value = 0x00;

	if (sdcard_readinit(sdcard_block_high, sdcard_block_mid, sdcard_block_low) == 0) { sdcard_ready = 0x00; }

	for (unsigned int i=0; i<512; i++) // packet of 512 bytes
	{
		// get value from SDcard
		temp_value = sdcard_receivebyte();
		
		sdcard_block_buffer[i] = temp_value;
	}

	if (sdcard_readfinal() == 0) { sdcard_ready = 0x00; }

	if (sdcard_ready == 0x00) { return 0; }
	else { return 1; }
}

unsigned char sdcard_writeinit(unsigned char high, unsigned char mid, unsigned char low)
{
	unsigned char temp_value = 0x00;

	sdcard_disable();
	sdcard_pump();
	sdcard_longdelay();
	sdcard_enable();
	sdcard_sendbyte(0x58); // CMD24 = 0x40 + 0x18 (24 in hex)
	sdcard_sendbyte(high);
	sdcard_sendbyte(mid);
	sdcard_sendbyte((low&0xFE)); // only blocks of 512 bytes
	sdcard_sendbyte(0x00);
	sdcard_sendbyte(0x01); // CRC (general)
	temp_value = sdcard_waitresult(); // command response
	if (temp_value == 0xFF) { return 0; }
	else if (temp_value != 0x00) { return 0; } // expecting 0x00
	sdcard_sendbyte(0xFE); // data packet starts with 0xFE

	return 1;
}

unsigned char sdcard_writefinal(void)
{
	unsigned char temp_value = 0x00;
	
	sdcard_sendbyte(0x55); // data packet ends with 0x55 then 0xAA
	sdcard_sendbyte(0xAA);
	temp_value = sdcard_receivebyte(); // toggle clock 8 times
	sdcard_disable();

	return 1;
}

unsigned char sdcard_writeblock()
{
	unsigned char temp_value = 0x00;

	if (sdcard_writeinit(sdcard_block_high, sdcard_block_mid, sdcard_block_low) == 0) { sdcard_ready = 0x00; }
	
	for (unsigned int i=0; i<512; i++) // packet of 512 bytes
	{
		temp_value = sdcard_block_buffer[i];

		// send to SDcard
		sdcard_sendbyte(temp_value);
	}

	if (sdcard_writefinal() == 0) { sdcard_ready = 0x00; }

	if (sdcard_ready == 0x00) { return 0; }
	else { return 1; }
}

unsigned char readkey(void)
{
	volatile unsigned char value = 0x00;
  
	if (keyreadpos != keywritepos)
	{
		if (keyarray[keyreadpos] == 0xF0) // release
		{
			keyrelease = 0x01;
		}
		else if (keyarray[keyreadpos] == 0xE0) // extended
		{
			keyextended = 0x01;
		}
		else if (keyarray[keyreadpos] == 0x12 || keyarray[keyreadpos] == 0x59) // shift
		{
			if (keyrelease == 0x01)
			{
				keyshift = 0x00;
				keyrelease = 0x00;
			}
			else
			{
				keyshift = 0x01;
			}
		}
		else if (keyarray[keyreadpos] == 0x58) // capslock
		{
			if (keyrelease == 0x01)
			{
				keycapslock = 0x01 - keycapslock;
				keyrelease = 0x00;
			}
		}
		else
		{
			if (keyrelease == 0x01)
			{
				keyrelease = 0x00;
				keyextended = 0x00;
			}
			else
			{
				value = conversion[(unsigned char)(keyarray[keyreadpos] + 
					0x80 * keyextended + 0x80 * keyshift + 0x80 * keycapslock)];
				
				keyextended = 0x00;
			}
		}
	
		keyreadpos++;
	}
	
	return value;
};


unsigned int readjoy(void) // many signals are done twice but only for safety
{ 
	volatile unsigned char joy_a = 0xFF;
	volatile unsigned char joy_b = 0xFF;
	volatile unsigned char i;
	volatile unsigned int both;
  
	//TRISB = 0xF0;
	LATB = 0x03;
	LATB = 0x07; // might interfere with TX/RX???
	LATB = 0x03;
	LATB = 0x0B;
	
	for (i=0x00; i<0x06; i++)
	{
		joy_a = (unsigned char)(joy_a << 1);
		if (RB5 != 0) joy_a = joy_a | 0x01;
		LATB = 0x0F;
		LATB = 0x0B;
	}
	
	for (i=0x00; i<0x06; i++)
	{
		joy_b = (unsigned char)(joy_b << 1);
		if (RB5 != 0) joy_b = joy_b | 0x01;
		LATB = 0x0F;
		LATB = 0x0B;
	}

	//TRISB = 0xE0;
	LATB = 0x03;
	LATB = 0x07;
	LATB = 0x03;
	LATB = 0x0B;
	
	for (i=0x00; i<0x04; i++)
	{
		LATB = 0x0F;
		LATB = 0x0B;
	}
	
	for (i=0x00; i<0x02; i++)
	{
		joy_a = (unsigned char)(joy_a << 1);
		if (RB5 != 0) joy_a = joy_a | 0x01;
		LATB = 0x0F;
		LATB = 0x0B;
	}
	
	for (i=0x00; i<0x04; i++)
	{
		LATB = 0x0F;
		LATB = 0x0B;
	}
	
	for (i=0x00; i<0x02; i++)
	{
		joy_b = (unsigned char)(joy_b << 1);
		if (RB5 != 0) joy_b = joy_b | 0x01;
		LATB = 0x0F;
		LATB = 0x0B;
	}
	
	//TRISB = 0xF0;
	LATB = 0x03;
	
	both = ((unsigned int)(joy_a) << 8) + (unsigned int)(joy_b);
	
	return both;
};

void highaddr(unsigned char value)
{
	LATC = value;
	LATB = 0x01;
	LATB = 0x03;
};

void lowaddr(unsigned char value)
{
	LATC = value;
};

void writedata(unsigned char value)
{
	PMDOUT1L = value;
	LATB = 0x02;
	LATB = 0x03;
};

unsigned char readdata(void) // requires highaddr() afterwards, works best if done twice
{ 
    PMSTATH = 0x00;
	LATB = 0x01;
	LATB = 0x00;
	LATB = 0x01;
	LATB = 0x01; // needs a second for delay purposes
	return PMDIN1L;
};

/*
void character(unsigned char x, unsigned char y, unsigned char value)
{
	volatile unsigned int pos = (unsigned int)(value) * 16;
	volatile unsigned char low = x * 2;
	volatile unsigned char high = y * 4;
	volatile unsigned char i = 0x00;
	
	highaddr(high);
	high++;
	lowaddr(low);
	low++;
  
	for (i=0; i<4; i++)
	{
		writedata(bitmap[pos]);
		pos++;
		lowaddr(low);
		low += 0x7F;
		writedata(bitmap[pos]);
		pos++;
		lowaddr(low);
		low++;
		writedata(bitmap[pos]);
		pos++;
		lowaddr(low);
		low = x * 2;
		writedata(bitmap[pos]);
		pos++;
		highaddr(high);
		high++;
		lowaddr(low);
		low++;
	}
};
*/

void character(unsigned char x, unsigned char y, unsigned char value)
{
	volatile unsigned int pos = 0x0000;
	volatile unsigned char low = x * 2;
	volatile unsigned char high = y * 4;
	volatile unsigned char i = 0x00;
	
	if (value >= 32) pos = (unsigned int)(value - 32) * 16;
	
	LATC = high;
	LATB = 0x01;
	LATB = 0x03;
	high++;
	LATC = low;
	low++;
  
	for (i=0; i<4; i++)
	{
		PMDOUT1L = bitmap[pos];
		LATB = 0x02;
		LATB = 0x03;
		pos++;
		LATC = low;
		low += 0x7F;
		PMDOUT1L = bitmap[pos];
		LATB = 0x02;
		LATB = 0x03;
		pos++;
		LATC = low;
		low++;
		PMDOUT1L = bitmap[pos];
		LATB = 0x02;
		LATB = 0x03;
		pos++;
		LATC = low;
		low = x * 2;
		PMDOUT1L = bitmap[pos];
		LATB = 0x02;
		LATB = 0x03;
		pos++;
		LATC = high;
		LATB = 0x01;
		LATB = 0x03;
		high++;
		LATC = low;
		low++;
	}
};

void invert(unsigned char x, unsigned char y)
{
	volatile unsigned char low = x * 2;
	volatile unsigned char high = y * 4; // need to change high addr to make it work right??
	volatile unsigned char i = 0x00;
	volatile unsigned char pixels;
  
	for (i=0; i<4; i++)
	{
		highaddr(high);
		lowaddr(low);
		pixels = readdata();
		highaddr(high);
		lowaddr(low);
		writedata((pixels ^ 0xFF));
		highaddr(high);
		lowaddr(low+0x01);
		pixels = readdata();
		highaddr(high);
		lowaddr(low+0x01);
		writedata((pixels ^ 0xFF));
		
		low += 0x80;
		
		highaddr(high);
		lowaddr(low);
		pixels = readdata();
		highaddr(high);
		lowaddr(low);
		writedata((pixels ^ 0xFF));
		highaddr(high);
		lowaddr(low+0x01);
		pixels = readdata();
		highaddr(high);
		lowaddr(low+0x01);
		writedata((pixels ^ 0xFF));
		
		low = x * 2;
		high++;
	}
};

void scroll(unsigned char x, unsigned char y)
{
	volatile unsigned char low = x * 2;
	volatile unsigned char high = y * 4; // need to change high addr to make it work right??
	volatile unsigned char i = 0x00;
	volatile unsigned char pixels;
  
	for (i=0; i<4; i++)
	{
		highaddr(high);
		lowaddr(low);
		pixels = readdata();
		highaddr((unsigned char)(high - 0x04));
		lowaddr(low);
		writedata(pixels);
		highaddr(high);
		lowaddr(low+0x01);
		pixels = readdata();
		highaddr((unsigned char)(high - 0x04));
		lowaddr(low+0x01);
		writedata(pixels);
		
		low += 0x80;
		
		highaddr(high);
		lowaddr(low);
		pixels = readdata();
		highaddr((unsigned char)(high - 0x04));
		lowaddr(low);
		writedata(pixels);
		highaddr(high);
		lowaddr(low+0x01);
		pixels = readdata();
		highaddr((unsigned char)(high - 0x04));
		lowaddr(low+0x01);
		writedata(pixels);
		
		low = x * 2;
		high++;
	}
};

void hex(unsigned char x, unsigned char y, unsigned char value)
{
	volatile unsigned char temp = (unsigned char)((value & 0xF0) >> 4);
		
	if (temp < 0x0A) character(x, y, (unsigned char)(temp + '0'));
	else character(x, y, (unsigned char)(temp - 0x0A + 'A'));
		
	temp = (unsigned char)(value & 0x0F);
		
	if (temp < 0x0A) character(x + 0x01, y, (unsigned char)(temp + '0'));
	else character(x + 0x01, y, (unsigned char)(temp - 0x0A + 'A'));
};

void decimal(unsigned char x, unsigned char y, unsigned int value)
{
	volatile unsigned int temp = 0;
	volatile unsigned int next = 0;
	
	next = value;
	
	temp = next / 10000;
	character(x, y, (unsigned char)(temp + '0'));
	next = next - 10000 * temp;
	
	temp = next / 1000;
	character(x + 0x01, y, (unsigned char)(temp + '0'));
	next = next - 1000 * temp;
	
	temp = next / 100;
	character(x + 0x02, y, (unsigned char)(temp + '0'));
	next = next - 100 * temp;
	
	temp = next / 10;
	character(x + 0x03, y, (unsigned char)(temp + '0'));
	next = next - 10 * temp;
	
	temp = next;
	character(x + 0x04, y, (unsigned char)(temp + '0'));
};

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/

const char map[448] = {
    // I
	' ', ' ', '*', ' ',
	' ', ' ', '*', ' ',
	' ', ' ', '*', ' ',
	' ', ' ', '*', ' ',
	
	' ', ' ', ' ', ' ',
	'*', '*', '*', '*',
	' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	' ', ' ', '*', ' ',
	' ', ' ', '*', ' ',
	' ', ' ', '*', ' ',
	' ', ' ', '*', ' ',
	
	' ', ' ', ' ', ' ',
	'*', '*', '*', '*',
	' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ',
  
	// J
	' ', ' ', '*', ' ',
	' ', ' ', '*', ' ',
	' ', '*', '*', ' ',
	' ', ' ', ' ', ' ',
	
	' ', ' ', ' ', ' ',
	' ', '*', ' ', ' ',
	' ', '*', '*', '*',
	' ', ' ', ' ', ' ',
	
	' ', ' ', ' ', ' ',
	' ', '*', '*', ' ',
	' ', '*', ' ', ' ',
	' ', '*', ' ', ' ',
	
	' ', ' ', ' ', ' ',
	'*', '*', '*', ' ',
	' ', ' ', '*', ' ',
	' ', ' ', ' ', ' ',
	
	// L
	' ', '*', ' ', ' ',
	' ', '*', ' ', ' ',
	' ', '*', '*', ' ',
	' ', ' ', ' ', ' ',
	
	' ', ' ', ' ', ' ',
	' ', '*', '*', '*',
	' ', '*', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	' ', ' ', ' ', ' ',
	' ', '*', '*', ' ',
	' ', ' ', '*', ' ',
	' ', ' ', '*', ' ',
	
	' ', ' ', ' ', ' ',
	' ', ' ', '*', ' ',
	'*', '*', '*', ' ',
	' ', ' ', ' ', ' ',
	
	// O
	' ', '*', '*', ' ',
	' ', '*', '*', ' ',
	' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	' ', '*', '*', ' ',
	' ', '*', '*', ' ',
	' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	' ', '*', '*', ' ',
	' ', '*', '*', ' ',
	' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	' ', '*', '*', ' ',
	' ', '*', '*', ' ',
	' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	// S
	' ', ' ', '*', '*',
	' ', '*', '*', ' ',
	' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	' ', '*', ' ', ' ',
	' ', '*', '*', ' ',
	' ', ' ', '*', ' ',
	' ', ' ', ' ', ' ',
	
	' ', ' ', '*', '*',
	' ', '*', '*', ' ',
	' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	' ', '*', ' ', ' ',
	' ', '*', '*', ' ',
	' ', ' ', '*', ' ',
	' ', ' ', ' ', ' ',
	
	// T
	' ', '*', ' ', ' ',
	'*', '*', '*', ' ',
	' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	' ', '*', ' ', ' ',
	' ', '*', '*', ' ',
	' ', '*', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	' ', ' ', ' ', ' ',
	'*', '*', '*', ' ',
	' ', '*', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	' ', '*', ' ', ' ',
	'*', '*', ' ', ' ',
	' ', '*', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	// Z
	'*', '*', ' ', ' ',
	' ', '*', '*', ' ',
	' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	' ', ' ', '*', ' ',
	' ', '*', '*', ' ',
	' ', '*', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	'*', '*', ' ', ' ',
	' ', '*', '*', ' ',
	' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ',
	
	' ', ' ', '*', ' ',
	' ', '*', '*', ' ',
	' ', '*', ' ', ' ',
	' ', ' ', ' ', ' ',
};



void solid(unsigned char x, unsigned char y, unsigned char value)
{
	volatile unsigned char low = x * 2;
	volatile unsigned char high = y * 4;
	volatile unsigned char i = 0x00;
	
	highaddr(high);
	high++;
	lowaddr(low);
	low++;
  
	for (i=0; i<4; i++)
	{
		writedata(value);
		lowaddr(low);
		low += 0x7F;
		writedata(value);
		lowaddr(low);
		low++;
		writedata(value);
		lowaddr(low);
		low = x * 2;
		writedata(value);
		highaddr(high);
		high++;
		lowaddr(low);
		low++;
	}
};

void block(unsigned char x, unsigned char y, unsigned char value)
{
	volatile unsigned char low = x * 2;
	volatile unsigned char high = y * 4;
	volatile unsigned char i = 0x00;
	
	highaddr(high);
	high++;
	lowaddr(low);
	low++;
  
	for (i=0; i<4; i++)
	{
		if (i == 0) writedata(0xFF);
		else writedata(value | 0xF0);
		lowaddr(low);
		low += 0x7F;
		if (i == 0) writedata(0xFF);
		else writedata(value);
		lowaddr(low);
		low++;
		writedata(value | 0xF0);
		lowaddr(low);
		low = x * 2;
		writedata(value);
		highaddr(high);
		high++;
		lowaddr(low);
		low++;
	}
};

void string(unsigned char x, unsigned char y, char *value)
{
	volatile unsigned char pos = 0;
	
	while (value[pos] != '\\')
	{
		character(x + pos, y, value[pos]);
		
		pos++;
	}
};

const unsigned char size_x = 10;
const unsigned char size_y = 25;
volatile unsigned char board[2*250];

volatile unsigned char pos_x[2]; // start at 4
volatile unsigned char pos_y[2]; // start at 4
volatile unsigned char rot[2];
volatile unsigned char new_pos_x[2];
volatile unsigned char new_rot[2];
volatile unsigned char piece[2];
volatile unsigned char new_piece[2];

volatile unsigned char test = 0;
volatile unsigned char count = 0;
volatile unsigned char seed = 0;

volatile unsigned char bag[14];
volatile unsigned char bag_pos[2];

volatile unsigned char speed[2];

volatile unsigned int lines[2];
volatile unsigned int timer[2];
	
volatile unsigned char key_value = 0x00;

volatile unsigned char joy_curr[2];
volatile unsigned char joy_prev[2];
volatile unsigned int joy_delay[2];

volatile unsigned char game_over[2];

void scratch(void)
{
	volatile unsigned char cursor_x = 0x00;
	volatile unsigned char cursor_y = 0x00;
  
	for (volatile unsigned char i=0; i<128; i++)
	{
		highaddr((unsigned char)(i));
		
		for (volatile unsigned char j=0; j<100; j++)
		{
			lowaddr((unsigned char)(j));
			writedata(0x00);
		}
		
		for (volatile unsigned char j=100; j<128; j++)
		{
			lowaddr((unsigned char)(j));
			writedata(0xB7); // 0xB7
		}
		
		for (volatile unsigned char j=128; j<228; j++)
		{
			lowaddr((unsigned char)(j));
			writedata(0x00);
		}
		
		for (volatile unsigned char j=228; j<255; j++)
		{
			lowaddr((unsigned char)(j));
			writedata(0xB7); // 0xB7
		}
	}
	
	invert(cursor_x, cursor_y);
			
	while (1)
	{	
		key_value = readkey();
				
		if (key_value != 0x00)
		{
			if (key_value == 0x1B) // escape (to clear)
			{
				for (volatile unsigned char i=0x00; i<0x20; i++)
				{
					for (volatile unsigned char j=0x00; j<0x32; j++)
					{
						character(j, i, ' ');
					}
				}
				
				cursor_x = 0x00;
				cursor_y = 0x00;
				
				invert(cursor_x, cursor_y);
			}
			else if (key_value == 0x0D) // return
			{
				invert(cursor_x, cursor_y);
				
				cursor_x = 0x00;
				
				if (cursor_y >= 0x1F) // scroll
				{
					for (volatile unsigned char i=0x01; i<0x20; i++)
					{
						for (volatile unsigned char j=0x00; j<0x32; j++)
						{
							scroll(j, i);
						}
					}
					
					for (volatile unsigned char i=0x00; i<0x32; i++)
					{
						character(i, 0x1F, ' ');
					}
					
					cursor_y = 0x1F;
				}
				else
				{
					cursor_y++;
				}
				
				invert(cursor_x, cursor_y);
			}
			else if (key_value == 0x08) // backspace
			{
				invert(cursor_x, cursor_y);
							
				if (cursor_x > 0x00)
				{
					cursor_x--;
				}
				
				invert(cursor_x, cursor_y);
			}
			else if (key_value == 0x09) // tab
			{
				invert(cursor_x, cursor_y);
				
				if (cursor_x < 0x31)
				{
					cursor_x++;
				}
						
				invert(cursor_x, cursor_y);
			}
			else if (key_value == 0x11) // up arrow
			{
				invert(cursor_x, cursor_y);
				
				if (cursor_y > 0x00)
				{
					cursor_y--;
				}
				
				invert(cursor_x, cursor_y);
			}
			else if (key_value == 0x12) // down arrow
			{
				invert(cursor_x, cursor_y);
				
				if (cursor_y < 0x1F)
				{
					cursor_y++;
				}
				
				invert(cursor_x, cursor_y);
			}
			else if (key_value == 0x13) // left arrow
			{
				invert(cursor_x, cursor_y);
				
				if (cursor_x > 0x00)
				{
					cursor_x--;
				}
				
				invert(cursor_x, cursor_y);
			}
			else if (key_value == 0x14) // right arrow
			{
				invert(cursor_x, cursor_y);
				
				if (cursor_x < 0x31)
				{
					cursor_x++;
				}
				
				invert(cursor_x, cursor_y);
			}
			else if (key_value == 0x01) // page up
			{
				invert(cursor_x, cursor_y);
				
				cursor_y = 0x00;
				
				invert(cursor_x, cursor_y);
			}
			else if (key_value == 0x02) // home
			{
				invert(cursor_x, cursor_y);
				
				cursor_x = 0x00;
				
				invert(cursor_x, cursor_y);
			}
			else if (key_value == 0x03) // end
			{
				invert(cursor_x, cursor_y);
				
				cursor_x = 0x31;
				
				invert(cursor_x, cursor_y);
			}
			else if (key_value == 0x04) // page down
			{
				invert(cursor_x, cursor_y);
				
				cursor_y = 0x1F;
				
				invert(cursor_x, cursor_y);
			}
			else if (key_value == 0x7F) // delete
			{
				invert(cursor_x, cursor_y);
				character(cursor_x, cursor_y, ' ');
				invert(cursor_x, cursor_y);
			}
			else if (key_value == 0x1A) // insert
			{
				// just for testing purposes
			}
			else if (key_value >= 0x20 && key_value < 0x7F) // letters
			{
				character(cursor_x, cursor_y, key_value);
				
				if (cursor_x < 0x31)
				{
					cursor_x++;
				}
				
				invert(cursor_x, cursor_y);
			}
			
		}
	}
};


void tetra()
{	
	for (volatile unsigned char i=0; i<128; i++)
	{
		highaddr((unsigned char)(i));
		
		for (volatile unsigned char j=0; j<100; j++)
		{
			lowaddr((unsigned char)(j));
			if (i < 8 || i >= 120) writedata(0xAA);
			else writedata((unsigned char)(i + j)); //0xEE);
		}
		
		for (volatile unsigned char j=100; j<128; j++)
		{
			lowaddr((unsigned char)(j));
			if (i < 8 || i >= 120) writedata(0xEF); // 0xB7
			else writedata(0x00);
		}
		
		for (volatile unsigned char j=128; j<228; j++)
		{
			lowaddr((unsigned char)(j));
			if (i < 8 || i >= 120) writedata(0xAA);
			else writedata((unsigned char)(i + j)); //0xEE);
		}
		
		for (volatile unsigned char j=228; j<255; j++)
		{
			lowaddr((unsigned char)(j));
			if (i < 8 || i >= 120) writedata(0xEF); // 0xB7
			else writedata(0x00);
		}
	}
	
	string(0x16, 0x00, "Tetra!\\");
	
	for (volatile unsigned char z=0; z<2; z++)
	{
		pos_x[z] = 7; // start at 4
		pos_y[z] = 4; // start at 4
		rot[z] = 0;
		new_pos_x[z] = 7;
		new_rot[z] = 0;
		piece[z] = 0;
		new_piece[z] = 1;
		
		for (volatile unsigned char i=0; i<7; i++)
		{
			bag[i + z * 7] = i;
		}
		
		bag_pos[z] = 1;
		
		speed[z] = 0; // max of 37

		lines[z] = 0;
		timer[z] = 0;

		joy_curr[z] = 0x00;
		joy_prev[z] = 0x00;
		joy_delay[z] = 0;
		
		game_over[z] = 0x01;

		for (volatile unsigned char i=0; i<size_y; i++)
		{
			for (volatile unsigned char j=0; j<size_x; j++)
			{
				if (i != size_y - 1)
				{
					board[j + i * size_x + z * size_x * size_y] = ' '; // empty
				}
				else
				{
					board[j + i * size_x + z * size_x * size_y] = '_'; // permanent bottom
				}
			}
		}
	}
	
	while (1)
	{	
		key_value = readkey();
		
		if (key_value == 0x1B) // escape
		{
			return;
		}
		
		unsigned volatile int joy_both = readjoy();
		
		joy_prev[0] = joy_curr[0];
		joy_curr[0] = (unsigned char)((joy_both & 0xFF00) >> 8);
		
		joy_prev[1] = joy_curr[1];
		joy_curr[1] = (unsigned char)((joy_both & 0x00FF));
		
		seed++; // random seed
		
		__delay_ms(15); // determines delay between calculations
		
		for (volatile unsigned char z=0; z<2; z++)
		{
			if (game_over[z] == 0)
			{
				for (volatile unsigned char i=0; i<size_y; i++)
				{
					for (volatile unsigned char j=0; j<size_x; j++)
					{
						if (board[j + i * size_x + z * size_x * size_y] == '*') 
						{
							board[j + i * size_x + z * size_x * size_y] = ' ';
						}
					}
				}
			}

			new_pos_x[z] = pos_x[z];
			new_rot[z] = rot[z];

			timer[z]++;

			if (timer[z] > 40 - speed[z]) // determines how fast it falls
			{
				timer[z] = 0;
			}

			joy_delay[z]++;

			if (joy_delay[z] > 2) // determines button turbo speed
			{
				joy_delay[z] = 0;
				joy_prev[z] = joy_prev[z] | 0xF0;
			}

			if ((((joy_curr[z] & 0x80) == 0x00) && ((joy_prev[z] & 0x80) == 0x80)) || 
				(key_value == 0x77 && z == 0) || (key_value == 0x69 && z == 1)) // up
			{
				timer[z] = 1; // not zero
				joy_delay[z] = 0;
			}
			else if ((((joy_curr[z] & 0x40) == 0x00) && ((joy_prev[z] & 0x40) == 0x40)) || 
				(key_value == 0x73 && z == 0) || (key_value == 0x6B && z == 1)) // down
			{
				timer[z] = 0;
				joy_delay[z] = 0;
			}
			else if ((((joy_curr[z] & 0x20) == 0x00) && ((joy_prev[z] & 0x20) == 0x20)) || 
				(key_value == 0x61 && z == 0) || (key_value == 0x6A && z == 1)) // left
			{
				new_pos_x[z]--;
				joy_delay[z] = 0;
			}
			else if ((((joy_curr[z] & 0x10) == 0x00) && ((joy_prev[z] & 0x10) == 0x10)) || 
				(key_value == 0x64 && z == 0) || (key_value == 0x6C && z == 1)) // right
			{
				new_pos_x[z]++;
				joy_delay[z] = 0;
			}
			else if ((((joy_curr[z] & 0x08) == 0x00) && ((joy_prev[z] & 0x08) == 0x08)) || 
				(key_value == 0x71 && z == 0) || (key_value == 0x75 && z == 1)) // button 1
			{
				if (game_over[z] != 0)
				{
					game_over[z] = 0;
					
					speed[z] = 0;
					
					lines[z] = 0;
					
					for (volatile unsigned char i=0; i<size_y; i++)
					{
						for (volatile unsigned char j=0; j<size_x; j++)
						{
							if (i != size_y - 1)
							{
								board[j + i * size_x + z * size_x * size_y] = ' '; // empty
							}
							else
							{
								board[j + i * size_x + z * size_x * size_y] = '_'; // permanent bottom
							}
						}
					}
				}
				else
				{  
					new_rot[z]++;
					if (new_rot[z] == 4) new_rot[z] = 0;
					joy_delay[z] = 0;
				}
			}
			else if ((((joy_curr[z] & 0x04) == 0x00) && ((joy_prev[z] & 0x04) == 0x04)) || 
				(key_value == 0x65 && z == 0) || (key_value == 0x6F && z == 1)) // button 2
			{
				if (game_over[z] != 0)
				{
					game_over[z] = 0;
					
					speed[z] = 0;
					
					lines[z] = 0;
					
					for (volatile unsigned char i=0; i<size_y; i++)
					{
						for (volatile unsigned char j=0; j<size_x; j++)
						{
							if (i != size_y - 1)
							{
								board[j + i * size_x + z * size_x * size_y] = ' '; // empty
							}
							else
							{
								board[j + i * size_x + z * size_x * size_y] = '_'; // permanent bottom
							}
						}
					}
				}
				else
				{
					if (new_rot[z] == 0) new_rot[z] = 3;
					else new_rot[z]--;
					joy_delay[z] = 0;
				}
			}
			
			if (game_over[z] != 0) continue;

			for (volatile unsigned char k=0; k<4; k++)
			{
				for (volatile unsigned char i=0; i<4; i++)
				{
					for (volatile unsigned char j=0; j<4; j++)
					{
						if (map[(unsigned int)j + (unsigned int)i*4 + (unsigned int)new_rot[z]*16 + (unsigned int)piece[z]*64] != ' ')
						{
							if (new_pos_x[z] + j >= 0x80 || new_pos_x[z] + j <= 0x03)
							{	
								new_pos_x[z]++;
								i = 5;
								j = 5;
							}
							else if (new_pos_x[z] + j >= size_x + 4)
							{
								new_pos_x[z]--;
								i = 5;
								j = 5;
							}
						}
					}
				}
			}
			
			test = 0;

			for (volatile unsigned char i=0; i<4; i++)
			{
				for (volatile unsigned char j=0; j<4; j++)
				{
					if (map[(unsigned int)j + (unsigned int)i*4 + (unsigned int)new_rot[z]*16 + (unsigned int)piece[z]*64] == '*')
					{
						if (board[(new_pos_x[z] + j - 4) + (pos_y[z] + i - 4) * size_x + z * size_x * size_y] != ' ')
						{
							test = 1;
							i = 5;
							j = 5;
						}
					}
				}
			}

			if (test == 0)
			{
				pos_x[z] = new_pos_x[z];
				rot[z] = new_rot[z];
			}

			if (timer[z] == 0)
			{
				pos_y[z]++;

				test = 0;

				for (volatile unsigned char i=0; i<4; i++)
				{
					for (volatile unsigned char j=0; j<4; j++)
					{
						if (map[(unsigned int)j + (unsigned int)i*4 + (unsigned int)rot[z]*16 + (unsigned int)piece[z]*64] == '*')
						{
							if (board[(pos_x[z] + j - 4) + (pos_y[z] + i - 4) * size_x + z * size_x * size_y] != ' ')
							{
								test = 1;
								i = 5;
								j = 5;
							}
						}
					}
				}

				if (test == 1)
				{	
					pos_y[z]--;

					for (volatile unsigned char i=0; i<4; i++)
					{
						for (volatile unsigned char j=0; j<4; j++)
						{
							if (map[(unsigned int)j + (unsigned int)i*4 + (unsigned int)rot[z]*16 + (unsigned int)piece[z]*64] == '*')
							{
								board[(pos_x[z] + j - 4) + (pos_y[z] + i - 4) * size_x + z * size_x * size_y] = '#';
							}
						}
					}
					
					sound(0xE000, 0x007F);

					if (pos_y[z] == 4)
					{
						game_over[z] = 1;
						
						sound(0xD000, 0x01FF);
					}
					else
					{
						count = 0;

						for (volatile unsigned char i=size_y-1; i>=1; i--)
						{
							test = 0;

							for (volatile unsigned char j=0; j<size_x; j++)
							{
								if (board[j + (i-1) * size_x + z * size_x * size_y] == ' ') test = 1;
							}

							if (test == 0)
							{
								for (volatile unsigned char j=i; j>=2; j--)
								{
									for (volatile unsigned char k=0; k<size_x; k++)
									{
										board[k + (j-1) * size_x + z * size_x * size_y] = 
											board[k + (j-2) * size_x + z * size_x * size_y];
									}
								}

								i++; // test that row again

								lines[z]++;
								count++;
								
								sound(0xF000, 0x01FF);
							}
						}

						if (game_over[1-z] != 0) count = 0;

						if (count >= 1) count--;

						for (volatile unsigned char i=0; i<count; i++)
						{
							for (volatile unsigned char j=0; j<size_y-1; j++)
							{
								for (volatile unsigned char k=0; k<size_x; k++)
								{
									if (j < size_y-2)
									{
										board[k + j * size_x + (1-z) * size_x * size_y] = 
											board[k + (j+1) * size_x + (1-z) * size_x * size_y];
									}
									else
									{
										if (k == seed % 10) board[k + j * size_x + (1-z) * size_x * size_y] = ' ';
										else board[k + j * size_x + (1-z) * size_x * size_y] = '#';
									}
								}
							}
						}
						
						pos_y[1-z] -= count;
						if (pos_y[1-z] < 4) pos_y[1-z] = 4;
					
						speed[z] = (unsigned char)(lines[z] / 10);
						if (speed[z] > 37) speed[z] = 37; // max

						pos_x[z] = 7; // start at 4
						pos_y[z] = 4; // start at 4
						rot[z] = 0;
						piece[z] = new_piece[z];

						bag_pos[z]++;

						if (bag_pos[z] == 7)
						{
							bag_pos[z] = 0;

							for (volatile unsigned char i=0; i<7; i++)
							{
								bag[(i + ((seed & 0xF0) >> 4)) % 7 + z * 7] = random[i + (seed & 0x1F) * 7];
							}
						}

						new_piece[z] = bag[bag_pos[z] + z * 7];
					}
				}
			}

			for (volatile unsigned char i=0; i<4; i++)
			{
				for (volatile unsigned char j=0; j<4; j++)
				{
					if (map[(unsigned int)j + (unsigned int)i*4 + (unsigned int)rot[z]*16 + (unsigned int)piece[z]*64] == '*')
					{
						board[(pos_x[z] + j - 4) + (pos_y[z] + i - 4) * size_x + z * size_x * size_y] = 
							map[(unsigned int)j + (unsigned int)i*4 + (unsigned int)rot[z]*16 + (unsigned int)piece[z]*64];
					}
				}
			}
		}
					
		for (volatile unsigned char i=0; i<size_y-1; i++)
		{
			for (volatile unsigned char j=0; j<size_x; j++)
			{
				switch (board[j + i * size_x])
				{
					case ' ':
					{
						solid(j + 0x08, i + 0x02, 0x00); // black
						break;
					}
					case '*':
					{
						if (piece[0] == 0) block(j + 0x08, i + 0x02, 0x66); // cyan
						else if (piece[0] == 1) block(j + 0x08, i + 0x02, 0x33); // blue
						else if (piece[0] == 2) block(j + 0x08, i + 0x02, 0xEE); // grey
						else if (piece[0] == 3) block(j + 0x08, i + 0x02, 0xCC); // yellow
						else if (piece[0] == 4) block(j + 0x08, i + 0x02, 0x55); // green
						else if (piece[0] == 5) block(j + 0x08, i + 0x02, 0xAA); // magenta
						else if (piece[0] == 6) block(j + 0x08, i + 0x02, 0x99); // red
						break;
					}
					case '#':
					{
						block(j + 0x08, i + 0x02, 0x11); // dark grey
						break;
					}
				}
				
				//character(j + 0x08, i, board[j + i * size_x + 0]);
			}
		}
		
		decimal(0x08, 0x00, lines[0]);
		
		if (new_piece[0] == 0) character(0x11, 0x00, 'I');
		else if (new_piece[0] == 1) character(0x11, 0x00, 'J');
		else if (new_piece[0] == 2) character(0x11, 0x00, 'L');
		else if (new_piece[0] == 3) character(0x11, 0x00, 'O');
		else if (new_piece[0] == 4) character(0x11, 0x00, 'S');
		else if (new_piece[0] == 5) character(0x11, 0x00, 'T');
		else if (new_piece[0] == 6) character(0x11, 0x00, 'Z');
		
		for (volatile unsigned char i=0; i<size_y-1; i++)
		{
			for (volatile unsigned char j=0; j<size_x; j++)
			{
				switch (board[j + i * size_x + size_x * size_y])
				{
					case ' ':
					{
						solid(j + 0x20, i + 0x02, 0x00); // black
						break;
					}
					case '*':
					{
						if (piece[1] == 0) block(j + 0x20, i + 0x02, 0x66); // cyan
						else if (piece[1] == 1) block(j + 0x20, i + 0x02, 0x33); // blue
						else if (piece[1] == 2) block(j + 0x20, i + 0x02, 0xEE); // grey
						else if (piece[1] == 3) block(j + 0x20, i + 0x02, 0xCC); // yellow
						else if (piece[1] == 4) block(j + 0x20, i + 0x02, 0x55); // green
						else if (piece[1] == 5) block(j + 0x20, i + 0x02, 0xAA); // magenta
						else if (piece[1] == 6) block(j + 0x20, i + 0x02, 0x99); // red
						break;
					}
					case '#':
					{
						block(j + 0x20, i + 0x02, 0x11); // dark grey
						break;
					}
				}
				
				//character(j + 0x20, i, board[j + i * size_x + size_x * size_y]);
			}
		}
		
		decimal(0x20, 0x00, lines[1]);
		
		if (new_piece[1] == 0) character(0x29, 0x00, 'I');
		else if (new_piece[1] == 1) character(0x29, 0x00, 'J');
		else if (new_piece[1] == 2) character(0x29, 0x00, 'L');
		else if (new_piece[1] == 3) character(0x29, 0x00, 'O');
		else if (new_piece[1] == 4) character(0x29, 0x00, 'S');
		else if (new_piece[1] == 5) character(0x29, 0x00, 'T');
		else if (new_piece[1] == 6) character(0x29, 0x00, 'Z');
		
		//hex(0x2E, 0x00, (unsigned char)((joy_curr & 0xFF00) >> 8));
		//hex(0x30, 0x00, (unsigned char)(joy_curr & 0x00FF));

		//if (key_value != 0x00)
		//{
		//	hex(0x2C, 0x00, key_value);
		//}
		
		if (game_over[0] != 0x00)
		{
			string(0x07, 0x1F, "Press Button\\");
		}
		else
		{
			string(0x07, 0x1F, "            \\");
		}
		
		if (game_over[1] != 0x00)
		{
			string(0x1F, 0x1F, "Press Button\\");
		}
		else
		{
			string(0x1F, 0x1F, "            \\");
		}
	}
};




unsigned char sdcard_splash(unsigned char skip)
{
	unsigned char pos = 0x00;
  
	if (sdcard_ready == 0x00)
	{
		for (unsigned char i=0; i<5; i++) // try 5 times to initialize
		{
			if (sdcard_initialize()) break; // break if it's initialized
		}
	}
	
	if (sdcard_ready != 0x00)
	{
		sdcard_block_low = 0x80 * skip;
		sdcard_block_mid = 0x00;
		sdcard_block_high = 0x00;
		
		for (unsigned char i=0; i<64; i++)
		{
			sdcard_readblock();
			
			highaddr(pos);
			pos++;
			
			for (unsigned int j=0; j<256; j++)
			{
				lowaddr(j);
				writedata(sdcard_block_buffer[j]);
			}
			
			highaddr(pos);
			pos++;
			
			for (unsigned int j=0; j<256; j++)
			{
				lowaddr(j);
				writedata(sdcard_block_buffer[(unsigned int)(j+256)]);
			}
			
			sdcard_block_low += 2;
			
			if (sdcard_block_low == 0x00)
			{
				sdcard_block_mid++;
				
				if (sdcard_block_mid == 0x00)
				{
					sdcard_block_high++;
				}
			}
		}
	}
	
	return sdcard_ready;
};

unsigned char sdcard_copy()
{
	unsigned char table_high = 0x00;
	unsigned char table_low = 0x00;
  
	if (sdcard_ready == 0x00)
	{
		for (unsigned char i=0; i<5; i++) // try 5 times to initialize
		{
			if (sdcard_initialize()) break; // break if it's initialized
		}
	}
	
	if (sdcard_ready != 0x00)
	{
		sdcard_block_low = 0x00; // starting after splash
		sdcard_block_mid = 0x01;
		sdcard_block_high = 0x00;
		
		for (unsigned char i=0; i<63; i++) // last 1K cannot be erased, so thus cannot be used here
		{
			TBLPTRU = 0x01;
			TBLPTRH = table_high;
			TBLPTRL = table_low;

			EECON1bits.WREN = 1;    // Enable write to memory
			EECON1bits.FREE = 1;    // Enable Erase operation
			INTCONbits.GIE = 0;     // Disable interrupts

			EECON2 = 0x55;
			EECON2 = 0xAA;

			EECON1bits.WR = 1;      // Clear the flash
			INTCONbits.GIE = 1;     // enable interrupts

			asm("NOP");             // Stall
			
			for (unsigned char j=0; j<2; j++)
			{	
				sdcard_readblock();
				
				for (unsigned char k=0; k<8; k++)
				{	
					for (unsigned char r = 0; r < 8; r++)
					{
						TBLPTRU = 0x01;
						TBLPTRH = table_high;
						TBLPTRL = table_low;

						for (unsigned char c = 0; c < 8; c++)
						{
							TABLAT = sdcard_block_buffer[(c + r * 8) + k * 64];

							asm("TBLWT*+");      // Push byte and then inc to next internal buffer cell
						}

						asm("TBLRD*-");         // Point back to original row

						EECON1bits.WREN = 1;    // Enable write to memory
						INTCONbits.GIE = 0;     // Disable interrupts

						EECON2 = 0x55;
						EECON2 = 0xAA;

						EECON1bits.WR = 1;      // Start programming flash
						INTCONbits.GIE = 1;     // Re-enable interrupts
						EECON1bits.WREN = 0;    // Disable write to memory

						table_low += 0x08;

						if (table_low == 0x00)
						{
							table_high += 0x01;
						}
					}
				}

				sdcard_block_low += 2;

				if (sdcard_block_low == 0x00)
				{
					sdcard_block_mid++;

					if (sdcard_block_mid == 0x00)
					{
						sdcard_block_high++;
					}
				}
			}
		}
	}
	
	return sdcard_ready;
};
		

void main(void)
{
	
  
    /* TODO <INSERT USER APPLICATION CODE HERE> */

	__delay_ms(1000); // delay at beginning until everything is stabilized
	
	// setup initial tri-state and output levels
	PORTA = 0x00;
	LATA = 0x00;
    TRISA = 0x00;
	PORTB = 0x03;
	LATB = 0x00;
	TRISB = 0xF0;
	PORTC = 0x00;
	LATC = 0x00;
	TRISC = 0x00;
	PORTD = 0x00;
	LATD = 0x00;
	TRISD = 0xFF;
	PORTE = 0x00;
	LATE = 0x00;
	TRISE = 0x0F; // 0xFF or 0xF0
	
	// disable interrupts
	INTCON = 0x00;
	INTCON2 = 0x80;
	INTCON3 = 0x00;
	PIE1 = 0x00;
	PIE2 = 0x00;
	PIE3 = 0x00;
	PIE4 = 0x00;
	PIE5 = 0x00;
	
	// enable interrupts for keyboard clock
	IPEN = 1; // enable interrupt priorities
	RBIP = 1; // high priority for RB
	RBIE = 1; // enable interrupt for RB
	GIEH = 1; // enables high priority interrupts
	
	// enable Timer0 and interrupts
	T0CON = 0x0F;
	TMR0IP = 1;
	TMR0IF = 0;
	TMR0IE = 1;
	TMR0 = 0xC000;
	TMR0ON = 1;
	
	sound(0x0000, 0x0000); // turn off sound
	
	
	
	// disable analog for digital I/O
	ANCON0 = 0xFF;
	ANCON1 = 0x1F;
	
	// disable open drain and use TTL on PMP
	ODCON1 = 0x00;
	ODCON2 = 0x00;
	ODCON3 = 0x00;
	PADCFG1 = 0x05; // 0x04 or 0x05
	
	// setup Parallel Master/Slave Port
	PMCONH = 0x00; // disable everything
	PMCONL = 0x0C; // 0x0C or 0x04, low-enable
	PMMODEH = 0x00; 
	PMMODEL = 0x00;
    
	PMEH = 0x00;
	PMEL = 0x00;
	PMADDRH = 0x40; // 0x00 or 0x40?
	PMADDRL = 0x00;
	PMSTATH = 0x00;
	PMSTATL = 0x00;
	
	PMCONH = 0x87; // 0x87, re-enable everything
	PMCONL = 0x84; // 0x80?
	
	
	// clear screen
	for (volatile unsigned char i=0; i<128; i++)
	{
		highaddr((unsigned char)(i));
		
		for (volatile unsigned char j=0; j<100; j++)
		{
			lowaddr((unsigned char)(j));
			writedata(0x00);
		}
		
		for (volatile unsigned char j=100; j<128; j++)
		{
			lowaddr((unsigned char)(j));
			writedata(0xB7); // 0xB7
		}
		
		for (volatile unsigned char j=128; j<228; j++)
		{
			lowaddr((unsigned char)(j));
			writedata(0x00);
		}
		
		for (volatile unsigned char j=228; j<255; j++)
		{
			lowaddr((unsigned char)(j));
			writedata(0xB7); // 0xB7
		}
	}
	
	
	sdcard_splash(0x00);
	
	__delay_ms(5000);
	
	sdcard_splash(0x01);
	
	sdcard_copy();
	
	tmr0value = 0xABCD;
	tmr0counter = 0x01FF; // TEMPORARY!
		
	extendedmemory();
	
	tetra();
	
	scratch();
}
	
