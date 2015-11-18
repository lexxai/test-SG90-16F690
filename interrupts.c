/******************************************************************************/
/*Files to Include                                                            */
/******************************************************************************/

#if defined(__XC)
#include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
#include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */
#include "user.h"

/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/

/* Baseline devices don't have interrupts. Note that some PIC16's 
 * are baseline devices.  Unfortunately the baseline detection macro is 
 * _PIC12 */
#ifndef _PIC12

// *  Author:  Gaurav
// *  website: www.circuitvalley.com 
// *  Email: mailchaduarygaurav@gmail.com
// *  https://github.com/circuitvalley/IR-Remote-Control

void interruptOnChangeIsr(void) {

    unsigned short long tdiff;
    unsigned char pin;
    static unsigned long rxbuffer;
    
    counterIOC++;

    tdiff = ((timer<<8)+TMR0); // calculate how much time has been passed since last interrupt 
    //send2BytesEUSART(TMR1H,TMR1L,false);
    // the time shold be less then time out and greater than PREPULSE 
    pin = IR_IN2; // store the current status of Sensor 
    TMR0 = 0; // reset the timer0 to measure the next edge(interrupt) of input
    timer = 0; // reset the timer varible to

    /* state machine is started here and it totally managed and keeps track of its states using the varible necpoj 
    here are the details of necpoj ( NEC position ) varible 
    if 
    necpoj == 1       we just detected the first edge of the input singal it may also mean(if interrupt is not false) that the 9ms leading pulse started 
                      after the first edge THE next pulse is expected to arrive around 9ms so the TIMEOUT is set to 11ms and PREPULSE is set to 8ms
				
    necpoj == 2 	  we just detected the second edge of the input signal and we finished the 9ms leding pulse and now 4.5ms space started 
                      after the second edge the next pulse is expected to arrive around 4.5ms so TIMEOUT is set to 5.5ms and PREPULSE is 3ms
		
    necpoj == 3  	  we just detected the third edge of the input singal and we finished 4.5ms space and addres lsb is now started 
                      after the third edge the next pulse is expected to arrive around 562.5us so TIMEOUT is set to 2.3ms and PREPULSE is 0.2ms (timeout can be much less at this state but to do this i have to add one more if else statemetnt)
		
    necpoj == 4 	  we just decected the forth edge and the 562.5 us burt of LSB of address has ended now a little space for '0'562.5us  or for '1' 1.6875ms   
                      after the forth edge the next pulse is expected to arrive for '0' around 562.5us  and for '1' 1.675ms so TIMEOUT is set to 2.3ms and PREPULSE is 0.2ms
		
    necpoj ==5 to 66  data pulse keep comming  
                        TIMOUT and PREPLUSE remain same as above.		   
			
    necpoj ==67		  we just fined the command inverse MSB space not the final 562.5us burst has stated  so we fined the receiveing 
                      now we will check the address and command for being correct
     */

    if ((tdiff > PREPULSE) && (tdiff < TIMEOUT)) // the edge (interrupt) occurrence time should be less then the TIMOUT and greater then PREPULESE else it is an fake singal
    { // At the very first edge (necpoj==0)  this conditon will always false and the false block of this if will bring the state machine (necpoj) to position 1(position 1 means 9ms leading pulse has started now we have to wait for 4.5ms start pulse to occur) 
        if (necpoj == 1 || necpoj == 2) // when we are hear it means 9ms leding pulse has ended and now we are necpoj=1 or necpoj=2
        {
            if ((pin == 1) && (necpoj == 1)) {
                necpoj++;
                TIMEOUT = TICKS5o5ms; // timeout for 3rd pulse 5.5ms	
                PREPULSE = TICKS3ms; // PREPULSE for 3rd pulse 3ms
            } else if ((pin == 0)&& (necpoj == 2)) {
                necpoj++;
                TIMEOUT = TICKS2o3ms; // now data starts so timeout is 2.3ms
                PREPULSE = TICKS0o2ms;
            } else // this block handle the conditon if any error occur after the completing the pre pulses 
            {
                necpoj = 0; //reset the state machine 
                TIMEOUT = TICKS11ms;
                PREPULSE = TICKS8ms;
            }
        } else if (necpoj > 2) //now we are picking the data 	
        {
            necpoj++; //necpoj sill inrement on every edge 	
            if (necpoj & 0x01) // here we check the if necpoj is an odd number because when necpoj goes greater then 3 then 
                //necpoj will always be and odd value when a single bit tranmission is over  
            {
                rxbuffer = rxbuffer << 1; //shift the buffer 
                if (tdiff > 1250) //we are here means we just recevied the edge of finished tranmission of a bit 
                    // so if last edge was more than 1.24 ms then the bit which is just over is one else it is zero 
                {
                    rxbuffer = rxbuffer | 0x1;
                    //	GPIObits.GPIO5 = !GPIObits.GPIO5;    
                } else {
                    rxbuffer = rxbuffer | 0x0;
                    //	GPIObits.GPIO4 = !GPIObits.GPIO4;
                }
            }
            if (necpoj > 66) // we have reached (Leading pulse 2 +address 16+~address16+ command 16+ ~command 16+ last final burst first edge 1)=67th edge of the message frame means the date tranmission is now over 
            {
                address = (rxbuffer >> 24)& 0xFF; //extract the data from the buffer 
                notaddress = (rxbuffer >> 16)& 0xFF;
                command = (rxbuffer >> 8) & 0xFF;
                notcommand = (rxbuffer) & 0xFF;
                rxbuffer = 0; //clear the buffer	
                if ((!(address & notaddress)) && (!(command & notcommand))) // check weather the received data is vaild or not
                {
                    dataready = 1;
                } else {
                    dataready = 0;
                }
                TIMEOUT = TICKS11ms; // weather we received the vaild data or not we have to reset the state machine 
                PREPULSE = TICKS8ms;
                necpoj = 0;
            }
        } else {
            TIMEOUT = TICKS11ms; // some error occured reset state machine 
            PREPULSE = TICKS8ms;
        }
    } else {
        if (pin == 0) //we are here means that after a longtimeout or PREPULSE we just detect a pulse which may be the start of 9ms pulse 
        {
            necpoj = 1; // yes it could be the start of 9ms pulse 
        } else {
            necpoj = 0; // no it's not start of 9ms pulse 
        }
//        address = 0xFF;
//        notaddress = 0xFF;
//        command = 0xFF;
//        notcommand = 0xFF;
        //dataready = 0x000;
        TIMEOUT = TICKS11ms; //default timing  
        PREPULSE = TICKS8ms;
    }
}

void interrupt isr(void) {
    /* This code stub shows general interrupt handling.  Note that these
    conditional statements are not handled within 3 seperate if blocks.
    Do not use a seperate if block for each interrupt flag to avoid run
    time errors. */

#if 1

    if (PIR1bits.CCP1IF) //Captre event of PWM modeule
    {
        IR_OUTPUT = !IR_OUTPUT; //software toogle of CCP1 Output pin
        IR_OUTPUT_FLUSH;
        //TMR1=0x0000;
        PIR1bits.CCP1IF = 0;
    } else if (INTCONbits.T0IF){
        //Timer0 overflow
        if(timer<0xFFFF)  timer++;	// this code is to increment the variable timer's value on every over flow but this if conditon will prevent this variable form rollover when a long timeout occurs
        INTCONbits.T0IF=0;
//    } else if (PIR1bits.TMR1IF) {   
//       //Timer1 overflow    
//        timer1ready=false;
//        PIR1bits.TMR1IF=0;
    } else if (INTCONbits.RABIF) // check the interrupt on change flag
    {
//        LED_SIGNAL = LED_SIGNAL_ON; // to blink the LED when IR signal is received ;									// to blink the LED when IR signal is received 
//        LED_SIGNAL_FLUSH;
        interruptOnChangeIsr(); // interrupt on change has been detected call the isr	
//        LED_SIGNAL = LED_SIGNAL_OFF; // to blink the LED when IR signal is received ;									// to blink the LED when IR signal is received 
//        LED_SIGNAL_FLUSH;
        INTCONbits.RABIF = 0; // clear the interrupt on chage flag
    }

#endif
}



#endif


