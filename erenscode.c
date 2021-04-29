#include <REG51.H>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BIT_ISSET(var, pos) (!!((var) & (1<<(pos))))
#define SET_BIT(var,pos) (var |=  (1<<pos))
#define CLR_BIT(var,pos) (var &= ~(1<<pos))
#define BUFFER_SIZE 3
#define limit 2
//LCD config
sbit DB7 = P1^7;
sbit DB6 = P1^6;
sbit DB5 = P1^5;
sbit DB4 = P1^4;
sbit RS = P1^3;
sbit E = P1^2;

sbit clear = P2^4;
sbit ret = P2^5;				  
sbit left = P2^6;
sbit right = P2^7;

bit keypad_flag,recieved_flag,setfreqflag,setampflag,dac_flag,disfreqflag,disampflag;
int key_val,buffer_index,freq,amp,i;
int tmp_var; 
int buffer[BUFFER_SIZE];
char serial_buffer[BUFFER_SIZE+1];
int serial_buffer_index;
unsigned int counter;
char var[16] ="                ";
unsigned int dly;


code char sin_lut[255] = {
0,3,6,9,12,15,18,21,24,27,30,34,37,39,42,45,48,51,54,57,
60,62,65,68,70,73,75,78,80,83,85,87,90,92,94,96,98,100,102,104,
105,107,109,110,112,113,115,116,117,118,119,120,121,122,123,124,124,125,125,126,
126,126,126,126,126,126,126,126,126,125,125,124,124,123,122,122,121,120,119,118,
116,115,114,112,111,109,108,106,104,103,101,99,97,95,93,91,88,86,84,82,
79,77,74,72,69,66,64,61,58,55,53,50,47,44,41,38,35,32,29,26,
23,20,17,14,10,7,4,1,-1,-4,-7,-10,-14,-17,-20,-23,-26,-29,-32,-35,
-38,-41,-44,-47,-50,-53,-55,-58,-61,-64,-66,-69,-72,-74,-77,-79,-82,-84,-86,-88,
-91,-93,-95,-97,-99,-101,-103,-104,-106,-108,-109,-111,-112,-114,-115,-116,-118,-119,-120,-121,
-122,-122,-123,-124,-124,-125,-125,-126,-126,-126,-126,-126,-126,-126,-126,-126,-126,-125,-125,-124,
-124,-123,-122,-121,-120,-119,-118,-117,-116,-115,-113,-112,-110,-109,-107,-105,-104,-102,-100,-98,
-96,-94,-92,-90,-87,-85,-83,-80,-78,-75,-73,-70,-68,-65,-62,-60,-57,-54,-51,-48,
-45,-42,-39,-37,-34,-30,-27,-24,-21,-18,-15,-12,-9,-6,-3}   ;



void nonblock_wait(int delay_time){ //creates variable delays of microsecond
	TR0=0;
	TMOD|= 0x01; //set timer 0 to mode 1: 16bit timer
	dly= 65535 - delay_time; // 2^16 = 65536 the number of states a 16bit timer can have
	TH0= dly>>8;
	TL0= dly&0x00FF;
	TR0=1;
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
	P0 = 0x7F;
	
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

bit getBit(char c, char bitNumber) {
	return (c >> bitNumber) & 1;
}

void delay(void) {
	char c;
	for (c = 0; c < 10; c++);
}
	
void returnHome(void) {
	RS = 0;
	DB7 = 0;
	DB6 = 0;
	DB5 = 0;
	DB4 = 0;
	E = 1;
	E = 0;
	DB5 = 1;
	E = 1;
	E = 0;
	delay();
}	

void entryModeSet(bit id, bit s) {
	RS = 0;
	DB7 = 0;
	DB6 = 0;
	DB5 = 0;
	DB4 = 0;
	E = 1;
	E = 0;
	DB6 = 1;
	DB5 = id;
	DB4 = s;
	E = 1;
	E = 0;
	delay();
}

void displayOnOffControl(bit display, bit cursor, bit blinking) {
	DB7 = 0;
	DB6 = 0;
	DB5 = 0;
	DB4 = 0;
	E = 1;
	E = 0;
	DB7 = 1;//mov
	DB6 = display;
	DB5 = cursor;
	DB4 = blinking;
	E = 1;
	E = 0;
	delay();
}

void cursorOrDisplayShift(bit sc, bit rl) {
	RS = 0;
	DB7 = 0;
	DB6 = 0;
	DB5 = 0;
	DB4 = 1;
	E = 1;
	E = 0;
	DB7 = sc;
	DB6 = rl;
	E = 1;
	E = 0;
	delay();
}

void functionSet(void) {
	// The high nibble for the function set is actually sent twice. Why? See 4-bit operation
	// on pages 39 and 42 of HD44780.pdf.
	DB7 = 0;
	DB6 = 0;
	DB5 = 1;
	DB4 = 0;
	RS = 0;
	E = 1;
	E = 0;
	delay();
	E = 1;
	E = 0;
	DB7 = 1;
	E = 1;
	E = 0;
	delay();
}

void setDdRamAddress(char address) {
	RS = 0;
	DB7 = 1;
	DB6 = getBit(address, 6);
	DB5 = getBit(address, 5);
	DB4 = getBit(address, 4);
	E = 1;
	E = 0;
	DB7 = getBit(address, 3);
	DB6 = getBit(address, 2);
	DB5 = getBit(address, 1);
	DB4 = getBit(address, 0);
	E = 1;
	E = 0;
	delay();
}

void sendChar(char c) {
	DB7 = getBit(c, 7);
	DB6 = getBit(c, 6);
	DB5 = getBit(c, 5);
	DB4 = getBit(c, 4);
	RS = 1;
	E = 1;
	E = 0;
	DB7 = getBit(c, 3);
	DB6 = getBit(c, 2);
	DB5 = getBit(c, 1);
	DB4 = getBit(c, 0);
	E = 1;
	E = 0;
	delay();
}

// -- End of LCD Module instructions
// --------------------------------------------------------------------

void sendString(char* str) {
	int index = 0;
	while (str[index] != 0) {
		sendChar(str[index]);
		index++;
	}
}

double sinosoid(int amp,int freq, int i){
	return amp *cos( (freq * i) % 360 );
}

void main (void){
	functionSet();
	entryModeSet(1, 0); // increment and no shift
	displayOnOffControl(1, 1, 1); // display on, cursor on and blinking on
	setupUART();
  ES  = 1; 		// ENABLE SERIAL INTERRUPT
	EX1 = 1; 		// ENABLE EXTERNAL INTERRUPT 1,	for keypad
	EA  = 1; 		// ENABLE INTERRUPTS
	IT1 = 1; 		// external interrupt on falling edge 
	PX1 = 1; 		// external interrupt higher prio
	TI = 1;
	P0  = 0x70; // initialise all keypad rows as 0 
	//P1 = 0x00; //DEBUGggggg
	keypad_flag = 0;
	recieved_flag= 0;
	setfreqflag = 0;
	setampflag = 0;
	buffer_index = 0;
	serial_buffer_index = 0;
	freq = 1;
	amp = 255;
	counter = 0;
	CLR_BIT(P0,7);

	nonblock_wait(1000);

	setDdRamAddress(0x00);
	sprintf(var,"A:%3d",amp);
	sendString(var);
	setDdRamAddress(0x40);
	sprintf(var,"F:%3d",freq);
	sendString(var);
	
	while(1){
		if (keypad_flag){
			EA  = 0;
			keypad_flag = 0;
			scan_keypad();
			P0  = 0x70;
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
					disfreqflag = 1;
					
					freq = tmp_var;
				}
				else if (key_val == 12){
					printf("amp:%d\n",tmp_var); //CHANGE THIS TO LCD
					amp = tmp_var;
					disampflag = 1;
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
							disampflag = 1;
							//sendString();
						}
						else if (setfreqflag){
							setfreqflag = 0;
							freq = tmp_var;
							printf("freq:%d\n",freq); //CHANGE THIS TO LCD
							disfreqflag = 1;
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
		
		
			if(TF0){
				TF0 = 0;
					P1 = amp*(sin_lut[(counter++)*(freq%255)])+(255/2);
				
					if(counter > 255){
						counter = 0;
					}
				nonblock_wait(1000);
			}
			
		if(disfreqflag){
			disfreqflag = 0;
			setDdRamAddress(0x40);
			sprintf(var,"F:%3d",freq);
			sendString(var);
		}
		if(disampflag){
			disampflag = 0;
			setDdRamAddress(0x00);
			sprintf(var,"A:%3d",amp);
			sendString(var);
		}
		
		if (ret == 0) {
			returnHome();
		}
		else {	
			if (left == 0 && right == 1) {
				cursorOrDisplayShift(1, 0); // shift display left
			}
			else if (left == 1 && right == 0) {
				cursorOrDisplayShift(1, 1); // shift display right
			}
				
		}
		
	}
	
}

