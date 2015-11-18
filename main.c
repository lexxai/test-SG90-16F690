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





//----------------------------------------------------

void main(void) {
    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize I/O and Peripherals for application */
    InitApp();

    uint16_t result;
    int8_t IrTry;
    //IR LED - ON
    disablePWMoutput;
    IR_OUTPUT = IR_OUTPUT_OFF;
    IR_OUTPUT_FLUSH;

    IrTry = IR_TRY;

    //    unsigned char *command;
    //    command[0] = 0x8D; // addres
    //    command[1] = 0xB1; // command

    send2BytesEUSART(0x55,0xAA,false);

    while (1) {
#if (use_IR_IN2_PWM_COMPARE)
        //Read analog value 
        delay_ms(5);
        ADCON0bits.GO_nDONE = 1;
        while (ADCON0bits.GO_nDONE);
        result = ADRESH << 8 | ADRESL;
        if ((result <= (LED_SIGNAL==LED_SIGNAL_OFF?IR_LIMIT1_HT:IR_LIMIT1_HB))
                                                       && result >= IR_LIMIT2) {
            if (IrTry > 0) IrTry--;
        } else {
            IrTry = IR_TRY;
        }
        LED_SIGNAL = (IrTry == 0) ? LED_SIGNAL_ON : LED_SIGNAL_OFF;
        LED_SIGNAL_FLUSH;
        //Send DWORD to UART
        //sendByteEUSART(IrTry==0);
        send2BytesEUSART(ADRESH, ADRESL);
        //sendByteEUSART(ADRESH);
        //sendByteEUSART(ADRESL);
#endif
#if (use_IR_IN2_PWM) 
        //send2BytesEUSART(0x80,0xDA,true);
        //delay_ms(1000);
        //send2BytesEUSART(TMR1H,TMR1L,true);
        if (dataready) // data is received and ready to be procssed 
        {
//            switch (command) // swich on 
//            {
//                case 0x50: RELAY1 = !RELAY1; //Toggle relay 1	
//                    break;
//                case 0xD8: RELAY2 = !RELAY2; //Toggle relay 2
//                    break;
//                case 0xF8: RELAY3 = !RELAY3; //Toggle relay 3
//                    break;
//                    //	case 0x30: RELAY4 = !RELAY4;		//Toggle relay 4		// can not use realy 4 as it is used for UART out
//                    //			   break;
//                case 0xB0: RELAY1 = 0; //Turn off all the relay
//                    RELAY2 = 0;
//                    RELAY3 = 0;
//                    //			   RELAY4 = 0;
//                    break;
//                default:
//                    break;
//            }
            dataready=0; 
            if (address==IRFRAME_ADRESS && command==IRFRAME_COMMAND){
                if (IrTry > 0)  IrTry--;
            }
//            sendByteEUSART(IrTry,true);
//            send2BytesEUSART(address,command,false);
        }else{
            IrTry = IR_TRY;
        }
        
        LED_SIGNAL = (IrTry == 0) ? LED_SIGNAL_ON : LED_SIGNAL_OFF;
        LED_SIGNAL_FLUSH;
        
        delay_ms(500);
        sendFrame(IRFRAME_ADRESS,IRFRAME_COMMAND);
        //send2BytesEUSART(0x80,0x6A,false);
        //delay_ms(1000);
        //delay_ms(800);
        
#endif  
        

    }

}

