volatile union {
    unsigned char byte;
    PORTAbits_t bits;
} LATPORTA;

#define LATPORTA_FLUSH PORTA = LATPORTA.byte

volatile union {
    unsigned char byte;
    PORTBbits_t bits;
} LATPORTB;

#define LATPORTB_FLUSH PORTB = LATPORTB.byte

volatile union {
    unsigned char byte;
    PORTCbits_t bits;
} LATPORTC;

#define LATPORTC_FLUSH PORTC = LATPORTC.byte



#define TRIS_OUTPUT 0
#define TRIS_INPUT 1
#define ANS_ANALOG 1
#define ANS_DIGITAL 0
#define IR_IN1 PORTCbits.RC7;
#define IR_IN1_ANS ANSELHbits.ANS9
#define IR_IN1_TRIS TRISCbits.TRISC7
#define IR_IN1_ANALOG_CHANNEL 9
#define IR_IN2 PORTBbits.RB4;
#define IR_IN2_ANS ANSELHbits.ANS10
#define IR_IN2_TRIS TRISBbits.TRISB4
#define IR_IN2_ANALOG_CHANNEL 10

#define IR_OUTPUT_TRIS TRISCbits.TRISC5
#define IR_OUTPUT LATPORTC.bits.RC5
#define IR_OUTPUT_ON  1
#define IR_OUTPUT_OFF  !IR_OUTPUT_ON
#define IR_OUTPUT_FLUSH LATPORTC_FLUSH

#define LED_SIGNAL_TRIS TRISBbits.TRISB6
#define LED_SIGNAL LATPORTB.bits.RB6
#define LED_SIGNAL_ON  1
#define LED_SIGNAL_OFF !LED_SIGNAL_ON
#define LED_SIGNAL_FLUSH LATPORTB_FLUSH

#define use_IR_IN1 0
#define use_IR_IN2_PWM_COMPARE 0
#define use_IR_IN2_PWM 1


#define enablePWMoutput  TMR2 = 0x00;CCP1CONbits.CCP1M=0b1100  //PWM mode active-high
#define disablePWMoutput CCP1CONbits.CCP1M = 0x0 //stop generating 38Khz singal. 

#define _t_delay_us(tu) TMR1=-tu;PIR1bits.TMR1IF=0;while(!PIR1bits.TMR1IF)
#define _t_delay_ms(t) for (int _loop_=t;_loop_>0;_loop_--) _t_delay_us(1000);


#define TICKS11ms 	11044      	// ticks in 11ms
#define TICKS5o5ms 	5522 		// ticks in 5.5ms
#define TICKS2o3ms 	2309 		// ticks in 2.3ms
#define TICKS3ms  	3012		// ticks in 3sm
#define TICKS0o2ms	200			// ticks in 0.2ms
#define TICKS8ms 	8032		// Tick

unsigned int TIMEOUT = TICKS11ms; // the pulse should occur before this time excede Otherwise it is an error 
unsigned int PREPULSE = TICKS8ms; // the interrupt should occur after this time Otherwise it is an error

volatile unsigned short long timer; // varible to keep track of long timeouts  ( it can also be int if you want to save flash memory for some other purpose  ) 
volatile unsigned char dataready; // varible to use as flag when data is completly received and  ready it is 1 else 0 

volatile unsigned char necpoj = 0; /* (necpoj=NEC position )this varible is used to keep track of the edges of the input singal 
											as decoding of the singal is done by a state machine 
											so this varible acutalley sotores what state we currently are 
							 				and total bits 32 and 2 leading pulse */


volatile unsigned char address = 0, notaddress = 0; // these varible are used to store received address
volatile unsigned char command = 0, notcommand = 0; // these varible are used to store received address

volatile bool timer1ready=true;
volatile unsigned char counterIOC;

/******************************************************************************/
/* User Level #define Macros                                                  */
/******************************************************************************/

/* TODO Application specific user parameters used in user.c may go here */

/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/

/* TODO User level functions prototypes (i.e. InitApp) go here */

void InitApp(void);         /* I/O and Peripheral Initialization */
void sendByteEUSART(unsigned char byte, bool sync);
void send2BytesEUSART(unsigned char byte1, unsigned char byte2, bool sync);
void delay_us(int tu);
void delay_ms(int t);
