#include <REG51.H>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BIT_ISSET(var, pos) (!!((var) & (1<<(pos))))
#define SET_BIT(var,pos) (var |=  (1<<pos))
#define CLR_BIT(var,pos) (var &= ~(1<<pos))
#define BUFFER_SIZE 3

bit keypad_flag,recieved_flag,setfreqflag,setampflag;
int key_val,buffer_index,freq,amp,i;
int tmp_var; 
int buffer[BUFFER_SIZE];
char serial_buffer[BUFFER_SIZE+1];
int serial_buffer_index;

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


void serial0() interrupt 4 {
	if (RI){
		RI = 0;
		recieved_flag = 1;
		serial_buffer[serial_buffer_index++] = SBUF;
	}
}
	


void main (void){
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
	recieved_flag= 0;
	setfreqflag = 0;
	setampflag = 0;
	buffer_index = 0;
	serial_buffer_index = 0;
	freq = 1;
	amp = 255;
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
				
				
				if (tmp_var > 255){
					printf("invalid input range\n");
					buffer_index = 0;
				}
				//assign to freq or amp depending on key
				else if (key_val == 10 ){
					printf("freq:%d\n",tmp_var); //CHANGE THIS TO LCD
					freq = tmp_var;
				}
				else if (key_val == 12){
					printf("amp:%d\n",tmp_var); //CHANGE THIS TO LCD
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
		
		if (recieved_flag){
			recieved_flag = 0;
					//handshake
					if ((setampflag || setfreqflag)&& (serial_buffer[serial_buffer_index-1] == '!')){
						tmp_var = atoi(serial_buffer);
						serial_buffer_index = 0;
						if (tmp_var > 255)
							printf("invalid input range\n"); 
						else if (setampflag){
							setampflag = 0;
							amp = tmp_var;
							printf("amp:%d\n",amp); //CHANGE THIS TO LCD
						}
						else if (setfreqflag){
							setfreqflag = 0;
							freq = tmp_var;
							printf("freq:%d\n",freq); //CHANGE THIS TO LCD
						}
					}
					else if ((setampflag || setfreqflag) && (serial_buffer[serial_buffer_index-1] < 48 || serial_buffer[serial_buffer_index-1] > 57)  ){
						printf("invalid input range\n");
						serial_buffer_index = 0;
						setampflag = 0;
						setfreqflag = 0;
					}					
					else if (serial_buffer[serial_buffer_index-1] == '*'){//set freq
						serial_buffer_index = 0;
						printf("ack\n"); //ack
						setfreqflag = 1;
					}
					else if (serial_buffer[serial_buffer_index-1] == '#'){//set amp
						serial_buffer_index = 0;
						printf("ack\n"); //ack
						setampflag = 1;
					}			
					else if (serial_buffer[serial_buffer_index-1] == 'f'){//get freq
						serial_buffer_index = 0;
						printf("ack\n"); //ack
						printf("%d\n",freq);
					}
					else if (serial_buffer[serial_buffer_index-1] == 'a'){//get amp
						serial_buffer_index = 0;
						printf("ack\n"); //ack
						printf("%d\n",amp);
					}
					else if ((!setampflag && !setfreqflag))
						printf("nack\n"); //ack
					
		}
		
		
	}
	
}

