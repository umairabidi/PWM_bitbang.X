/* 
 * File:   MotorControl.c
 * Author: Umair
 *
 * Created on July 29, 2023, 7:47 PM
 */

#include <stdio.h>
#include <stdlib.h>

/* PWM motor speed control  using PWM and a potentiometer to adjust setpoint.
 * Set PWM to Timer 2
 */

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include "../include/proc/pic16f630.h"

#pragma config FOSC = INTRCIO   // Oscillator Selection bits (INTOSC oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RA3/MCLR pin function select (RA3/MCLR pin function is MCLR)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config CP = OFF         // Code Protection bit (Program Memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)

#define _XTAL_FREQ 8000000

#define MotorPin PORTCbits.RC3      // Pin 7, output
#define ExtIntPin PORTAbits.RA5     // Pin 2, output
#define IOCPin PORTAbits.RA3        // Pin 4, input
#define IOCflag INTCONbits.RAIF     // To keep track of interrupt

#define encA PORTAbits.RA2   // Pin 
#define encB PORTCbits.RC0   // Pin 


uint8_t PWMCounter;
uint8_t th_h, th_l;     // High and low thresholds
volatile __bit PWMState;
volatile __bit enc_F;
volatile __bit dir;

void PWM_setup();
void rotary_setup();


void __interrupt() isr(void){
    if (INTCONbits.T0IF){
        INTCONbits.T0IF = 0;
        TMR0 = PWMState ? 255 - th_l : 255 - th_h;
        PWMState = PWMState^1;
        MotorPin = PWMState;
    }
    if (INTCONbits.RAIF){           // If the IOC has triggered
        INTCONbits.RAIF = 0;
        enc_F = 1;                  // Clear the flag bit
        dir = (encA + encB)%2;
    }
}

int main(int argc, char** argv) {
    PWM_setup();
    rotary_setup();
    
    enc_F = 0;
    th_h = 255;
    th_l = 0;
    
    TRISCbits.TRISC3 = 0;       // Motor output
    INTCONbits.GIE = 1;         // Global interrupt enable
    
    while(1){
        if (enc_F){
            enc_F = 0;
            th_l = th_l > 254 ? 254 : th_l;
            th_l = th_l < 1 ? 1 : th_l;
            th_l += dir*2-1;
            th_h = 255-th_l;
        }
    }
    return (EXIT_SUCCESS);
}

/* Start Timer0 = (255-th_h)
 * It counts up to 255 and overflows
 * Toggle the output bit
 * Set the new Timer0 = (255-th_l)
 * It counts up to 255 and overflows again
 * Toggle the bit
 * repeat
 */

void PWM_setup(){
    // Timer 0 setup
    OPTION_REGbits.T0CS = 0;    // TMR0 Clock Source Select bit = internal clk
    OPTION_REGbits.PSA = 0;     // Prescaler is assigned to the Timer0 module
    OPTION_REGbits.PS = 0b010;  // Prescaler
    // Interrupt setup
    INTCONbits.RAIE = 1;        // PORTA IOC enable
    INTCONbits.T0IE = 1;        // Timer0 overflow interrupt enable
}

void rotary_setup(){
    // IOC setup
    TRISAbits.TRISA2 = 1;       // encA
    TRISCbits.TRISC0 = 1;       // encB
    IOCAbits.IOCA2 = 1;         // IOC on encA
}