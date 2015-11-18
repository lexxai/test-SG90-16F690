/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
#include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
#include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */

/******************************************************************************/
/* User Global Variable Declaration                                           */
/******************************************************************************/

/* i.e. uint8_t <variable_name>; */

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/
#define IR_LIMIT1 990
#define IR_histeresis 20
#define IR_LIMIT1_HT IR_LIMIT1+IR_histeresis
#define IR_LIMIT1_HB IR_LIMIT1-IR_histeresis
#define IR_LIMIT2 20
#define IR_TRY   3





#define durationLogic1  1686  //sum 2232  //us
#define durationLogic0  561   //sum 1125  //us
#define durationBeacon 8999  //us
#define durationSpace  4490  //us
#define durationSpaceRepeat  2250  //us
#define durationRepeat 11174  //us

#define IRFRAME_ADRESS 0x10
#define IRFRAME_COMMAND 0x6A








// USERS SUB

void sendIRbit(bool b) {
    enablePWMoutput;
    delay_us(durationLogic0);
    disablePWMoutput;
    if (b) {
        delay_us(durationLogic1);
    } else {
        delay_us(durationLogic0);
    }
}

void sendIRByte(unsigned char byte) {
    for (int j = 8; j > 0; j--) {
        sendIRbit(byte & 0x80);
        byte = byte << 1;
    }
}

void sendIRServiceBit(bool type) {
    //type 0-prefix, 1-suffix
    enablePWMoutput;
    if (type) {
        delay_us(durationLogic0);
        disablePWMoutput;
        delay_ms(40); // wait for the Data Frame time. 
    } else {
        delay_us(durationBeacon); // leading PULSE
        disablePWMoutput;
        delay_us(durationSpace); // space
    }
}

void sendFrame(unsigned char address, unsigned char command) {
    sendIRServiceBit(0);
    sendIRByte(address);
    sendIRByte(~address);
    sendIRByte(command);
    sendIRByte(~command);
    sendIRServiceBit(1);
}

void sendRepeate() {
    enablePWMoutput;
    delay_us(durationBeacon); //wait for ~9ms 	
    disablePWMoutput;
    delay_us(durationSpaceRepeat); //wait for 2.25ms

    enablePWMoutput;
    delay_us(durationLogic0); //wait for ~562.5us
    disablePWMoutput;
    delay_us(96187); //delay for 96.187 ms before sending the next repeate code

}
#define servomin 544
#define servomax 2300
#define servowidth  servomax-servomin
#define perangle   10// (servowidth)/180

void servo(int degree) {
    uint16_t delay=servomin;
    delay = delay +(degree*perangle);
    //delay = servomin+(degree*perangle);
//    switch (degree) {
//        case 0:
//            delay = servomin;
//            break;
////        case 45:
////            delay = 1250;
////            break;
//        case 90:
//            delay = 1400;
//            break;
////        case 135:
////            delay = 1750;
////            break;
//        case 180:
//            delay = servomax;
//            break;
//        default:
//            delay = servomin+(degree*perangle);
//            break;
//    }
    for (int i = 0; i < 99; i++) {
        SERVO_OUTPUT = SERVO_OUTPUT_ON;
        SERVO_OUTPUT_FLUSH;
        delay_us(delay);
        SERVO_OUTPUT = SERVO_OUTPUT_OFF;
        SERVO_OUTPUT_FLUSH;
        delay_us(20000-delay);
    }
}


//----------------------------------------------------
#define step 45
void main(void) {
    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize I/O and Peripherals for application */
    InitApp();

    disablePWMoutput;
    SERVO_OUTPUT = SERVO_OUTPUT_OFF;
    SERVO_OUTPUT_FLUSH;
    delay_ms(1000);
    while (1) {
        /////// rotate to 0 degree  
        for (int i = 0; i < 180; i+=step) {
            servo(i);
        }
        for (int i = 180; i >0 ; i-=step) {
            servo(i);
        }

        
    }
}

