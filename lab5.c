#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include "mbed.h"


int len = 63;

//using namespace std;


struct music_struct {
    int length;
	char ** notalar_left;
	char **  notalar_right;
	int * not_indexes_left;
	int *not_indexes_right;
};

//int index_routine(char ***alphabet, char* x);
//char* substr(const char *src, int m, int n);



//int index_routine(const char* x);
PwmOut speaker(p21);
PwmOut speaker2(p22);
Ticker t1;
Ticker t2;
InterruptIn btn(BUTTON1);
InterruptIn btn2(BUTTON1);
EventQueue queue;

const char * notes[][3] = {{"C0",	"16.35", 	"2109.89"},
                        {"D0",	"18.35", 	"1879.69"},
                        {"E0",	"20.60", 	"1674.62"},
                        {"F0",	"21.83", 	"1580.63"},
                        {"G0",	"24.50", 	"1408.18"},
                        {"A0",	"27.50", 	"1254.55"},
                        {"B0",	"30.87", 	"1117.67"},
                        {"C1",	"32.70", 	"1054.94"},
                        {"D1",	"36.71", 	"939.85"},
                        {"E1",	"41.20", 	"837.31"},
                        {"F1",	"43.65", 	"790.31"},
                        {"G1",	"49.00", 	"704.09"},
                        {"A1",	"55.00", 	"627.27"},
                        {"B1",	"61.74", 	"558.84"},
                        {"C2",	"65.41", 	"527.47"},
                        {"D2",	"73.42", 	"469.92"},
                        {"E2",	"82.41", 	"418.65"},
                        {"F2",	"87.31", 	"395.16"},
                        {"G2",	"98.00", 	"352.04"},
                        {"A2",	"110.00", 	"313.64"},
                        {"B2",	"123.47", 	"279.42"},
                        {"C3",	"130.81", 	"263.74"},
                        {"D3",	"146.83", 	"234.96"},
                        {"E3",	"164.81", 	"209.33"},
                        {"F3",	"174.61", 	"197.58"},
                        {"G3",	"196.00", 	"176.02"},
                        {"A3",	"220.00", 	"156.82"},
                        {"B3",	"246.94", 	"139.71"},
                        {"C4",	"261.63", 	"131.87"},
                        {"D4",	"293.66", 	"117.48"},
                        {"E4",	"329.63", 	"104.66"},
                        {"F4",	"349.23", 	"98.79"},
                        {"G4",	"392.00", 	"88.01"},
                        {"A4",	"440.00", 	"78.41"},
                        {"B4",	"493.88", 	"69.85"},
                        {"C5",	"523.25", 	"65.93"},
                        {"D5",	"587.33", 	"58.74"},
                        {"E5",	"659.25", 	"52.33"},
                        {"F5",	"698.46", 	"49.39"},
                        {"G5",	"783.99", 	"44.01"},
                        {"A5",	"880.00", 	"39.20"},
                        {"B5",	"987.77", 	"34.93"},
                        {"C6",	"1046.50", 	"32.97"},
                        {"D6",	"1174.66", 	"29.37"},
                        {"E6",	"1318.51", 	"26.17"},
                        {"F6",	"1396.91", 	"24.70"},
                        {"G6",	"1567.98", 	"22.00"},
                        {"A6",	"1760.00", 	"19.60"},
                        {"B6",	"1975.53", 	"17.46"},
                        {"C7",	"2093.00", 	"16.48"},
                        {"D7",	"2349.32", 	"14.69"},
                        {"E7",	"2637.02", 	"13.08"},
                        {"F7",	"2793.83", 	"12.35"},
                        {"G7",	"3135.96", 	"11.00"},
                        {"A7",	"3520.00", 	"9.80"},
                        {"B7",	"3951.07", 	"8.73"},
                        {"C8",	"4186.01", 	"8.24"},
                        {"D8",	"4698.63", 	"7.34"},
                        {"E8",	"5274.04", 	"6.54"},
                        {"F8",	"5587.65", 	"6.17"},
                        {"G8",	"6271.93", 	"5.50"},
                        {"A8",	"7040.00", 	"4.90"},
                        {"B8",	"7902.13", 	"4.37"},
};

const char * music[][18]= {{"A0","10","B4","25","C0","12","A0","10","B4","25","C0","12","A0","10","B4","25","C0","12"},
                      {"A0","40","B4","15","C0","02","A0","60","B4","45","C0","22","A0","50","B4","55","C0","60"},};

void play_tone(float frequency, float volume, int interval) {
    speaker.period(1.0 / frequency);
    speaker = volume;
    wait(interval);
    speaker = 0.0;
    wait(2);
}
void play_tone2(float frequency, float volume, int interval) {
    speaker2.period(1.0 / frequency);
    speaker2 = volume;
    wait(interval);
    speaker2 = 0.0;
    wait(2);
}
int index_routine(const char* x){
    int index = -1;
    for(int i = 0; i < len; i++){
        
        if(strcmp(notes[i][0], x)==0){
            
            index = i;
            break;
        }
    }
    return index;
}
//char* substr(const char *src, int m, int n);
char* substr(const char *src, int m, int n){
	// get length of the destination string
	int len = n - m;

	// allocate (len + 1) chars for destination (+1 for extra null character)
	char *dest = (char*)malloc(sizeof(char) * (len + 1));

	// extracts characters between m'th and n'th index from source string
	// and copy them into the destination string
	for (int i = m; i < n && (*(src + i) != '\0'); i++)
	{
		*dest = *(src + i);
		dest++;
	}

	// null-terminate the destination string
	*dest = '\0';

	// return the destination string
	return dest - len;
}
//int index_routine(string **alphabet, string x);


void event_speaker2(void * arg){
    struct music_struct *arg_struct = ((struct music_struct *) arg);
     for(int i = 0; i < arg_struct->length/2; i++){
            printf("2222222222222222222222222222222222222222222222222222222222\n");
            printf("Sterio2:%s \nFrequency: %s\n,interval: %d\n",substr( arg_struct->notalar_right[i],i*2,i*2+2 ),notes[arg_struct->not_indexes_right[i]][1],substr( arg_struct->notalar_right[i],i*2+2,i*2+4 ));
            play_tone2(atof( notes[arg_struct->not_indexes_right[i]][1] ),atof( notes[arg_struct->not_indexes_right[i]][2] ), atoi (substr( arg_struct->notalar_right[i],i*2+2,i*2+4 ))/10);
            
        }
}
void event_speaker1(void * arg){
    struct music_struct *arg_struct = ((struct music_struct *) arg);
     for(int i = 0; i < arg_struct->length/2; i++){
            printf("1111111111111111111111111111111111111111111111111111111111\n");
            printf("Sterio1:%s \nFrequency: %s\n,interval: %s\n",substr( arg_struct->notalar_right[i],i*2,i*2+2 ) ,notes[arg_struct->not_indexes_left[i]][1],substr( arg_struct->notalar_left[i],i*2+2,i*2+4 ));
            play_tone(atof( notes[arg_struct->not_indexes_left[i]][1]),atof( notes[arg_struct->not_indexes_left[i]][2] ), atoi( substr( arg_struct->notalar_left[i],i*2+2,i*2+4 ) )/10);
            
            
        }
}
void repeat(void * arg){
    //struct music_struct *arg_struct = ((struct music_struct *) arg);
    
}
int main()
{
        

    

    
    //
    int length = 18;
    char  notalar_left[length][2];
    char  notalar_right[length][2];
    
    int * not_indexes_left;
    
    not_indexes_left = (int*)malloc((length/2) * sizeof(int));
    int * not_indexes_right = (int*)malloc((length/2) * sizeof(int));
    
    
    for(int i = 0; i < length; i++){
        strcpy(notalar_left[i], music[0][i]);
        strcpy(notalar_right[i], music[1][i]);
    }
    
    //printf("%s\n",music[0][0]);
    for(int i = 0; i < length/2; i = i+2){
        
        not_indexes_left[i] = index_routine(music[0][i]);
        not_indexes_right[i] = index_routine(music[1][i]);
    }

    /*    
    for(int i = 0; i < length-2; i++)
    {
        printf("\n Element is: %s \n",substr(notalar_left,i,i+2));
        printf("\n INdex is: %d \n",not_indexes_left[i]);
    }
    */
   // while(1){
    
    struct music_struct *all= (struct music_struct *)calloc(1 , sizeof(struct music_struct));
    all[0].length =length;
    all[0].notalar_left = (char **)malloc(length * sizeof(char *)); // Allocate row pointers
    all[0].notalar_right = (char **)malloc(length * sizeof(char *)); // Allocate row pointers
    for(int i = 0; i < length; i++){
        all[0].notalar_left[i] = (char *)malloc(2 * sizeof(char));  // Allocate each row separately
        all[0].notalar_right[i] = (char *)malloc(2 * sizeof(char));  // Allocate each row separately
    }
        
    for(int i = 0; i < length; i++){
        for(int j = 0; j < 2; j++){
            all[0].notalar_left[i][j] = notalar_left[i][j];
            all[0].notalar_right[i][j] = notalar_right[i][j];
        }    
    }
    all[0].not_indexes_left = (int*)malloc((length/2) * sizeof(int));
    all[0].not_indexes_right = (int*)malloc((length/2) * sizeof(int));
    for(int j = 0; j < length/2; j++){
        all[0].not_indexes_left[j] = not_indexes_left[j];
        all[0].not_indexes_right[j] = not_indexes_right[j];
    }
    //void *arg = all;
    //t1.attach(callback(&event_speaker1),(void *) (&all[0]));
    queue.call(&event_speaker1,(void *) (&all[0]) );
    queue.call(&event_speaker2,(void *) (&all[0]) );
    btn.fall( queue.event(&event_speaker1,(void *) (&all[0])) );
    btn2.fall( queue.event(&event_speaker2,(void *) (&all[0])) );
    //btn.fall(queue.event(&event_speaker2));

    // Because the simulator does not support multiple threads,
    // we have to call dispatch_forever from the main thread.
    // Typically you'd run this on a separate thread within Mbed's RTOS.
    queue.dispatch_forever();
    //}
    //btn.fall(callback(&toggle_led2));
    printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
}



