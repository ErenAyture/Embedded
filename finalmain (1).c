#include <REG51.H>
#include <stdio.h>
#include <math.h>

#define BIT_ISSET(var, pos) (!!((var) & (1<<(pos))))
#define SET_BIT(var,pos) (var |=  (1<<pos))
#define CLR_BIT(var,pos) (var &= ~(1<<pos))
#define BUFFER_SIZE 3

bit keypad_flag;
int key_val,buffer_index,freq,amp,i;
int tmp_var; 
int buffer[BUFFER_SIZE];

void setupUART();
bit column_scan_subroutine();
void scan_keypad();
void setup();


void main (void){
	
	setup();
	
	while(1){
		if (keypad_flag){
			EA  = 0;
			keypad_flag = 0;
			scan_keypad();
			P0  = 0xF0;
			IE1 = 0; //ignore the rising edge interrupt?
			
			if(key_val == 10 || key_val == 12 ){ //* and # respectively
				
				//convert key presses to a decimal
				tmp_var = 0;
				for (i = 0; i < buffer_index; i++)
					tmp_var += ((int)(pow(10,(buffer_index-i-1))+0.5))*buffer[i];
				
				//assign to freq or amp depending on key
				if (key_val == 10 ){
					printf("freq:%d\n",tmp_var);
					freq = tmp_var;
				}
				else if (key_val == 12){
					printf("amp:%d\n",tmp_var);
					amp = tmp_var;
				}
				
				//reset buffer
				buffer_index = 0;
			}
			else if (buffer_index < BUFFER_SIZE){
				if (key_val == 11) // 0 is eleventh key on keypad
					key_val = 0;
				buffer[buffer_index++] = key_val;
			}
			else {
				printf("invalid input range\n");
				buffer_index = 0;
			}
			
			EA  = 1;
		}
		
		
	}
	
}

void setup(){
	setupUART();
  ES  = 1; 		// ENABLE SERIAL INTERRUPT
	EX1 = 1; 		// ENABLE EXTERNAL INTERRUPT 1,	for keypad
	EA  = 1; 		// ENABLE INTERRUPTS
	IT1 = 1; 		// external interrupt on falling edge 
	PX1 = 1; 		// external interrupt higher prio
	TI = 1;
	P0  = 0xF0; // initialise all keypad rows as 0 
	P1 = 0x00; //DEBUGggggg
	keypad_flag = 0;
	buffer_index = 0;
}



void setupUART(){
	SCON  = 0x50;  // mod 1, reciever enabled
//	SCON  = 0xD0;  // mod 3, reciever enabled
	TMOD |= 0x20; //set timer 1 to mode 2: 8-\bit \auto reload
	PCON |= 0x00; //SMOD DISABLED
	TH1 = 0xFE; //TH1 = 256 - ((system frequency / (12 * 32)) / baud) with baud 19200 and system freq 14.7456Mhz SMOD 0
	TR1= 1;
}

void ex1_isr(void) interrupt 2 {
		keypad_flag = 1;
		P1++; //debug
}


bit column_scan_subroutine() {
	key_val++;
	if (!BIT_ISSET(P0,6))
		return 1;
	key_val++;
	if (!BIT_ISSET(P0,5))
		return 1;
	key_val++;
	if (!BIT_ISSET(P0,4))
		return 1;
	
	return 0;
}

void scan_keypad() {
	key_val =0;
	P0 = 0xFF;
	
	CLR_BIT(P0,3);
	if(column_scan_subroutine())
		return;
	
	SET_BIT(P0,3);
	CLR_BIT(P0,2);
	if(column_scan_subroutine())
		return; 
	
	SET_BIT(P0,2);
	CLR_BIT(P0,1);
	if(column_scan_subroutine())
		return;
	
	SET_BIT(P0,1);
	CLR_BIT(P0,0);
	if(column_scan_subroutine())
		return;
	
	key_val= 0;
	return;
}

	