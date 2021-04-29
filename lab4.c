#include "mbed.h"
#include "mbed_events.h"
#include "SimulatorBlockDevice.h"

#define BD_PAGE_SIZE               512
#define circular_buffer_size        10

// buffer to store the counter
float *page_buffer;
int *i;
uint32_t *page_counter;
uint32_t *page_old_counter;
uint32_t * circular_buffer;
Ticker flipper;
Ticker flipper2;


//DigitalOut led(p5);
PwmOut led(p5);

// Initialize a persistent block device with 512 bytes block size, and 128 blocks (64K of storage)
SimulatorBlockDevice bd("myblockdevice", 128 * BD_PAGE_SIZE, BD_PAGE_SIZE);
EventQueue queue;
InterruptIn btn(BUTTON1);

uint32_t max_buffer(uint32_t *arr);

void btn_fall() {
    /*
    // up the counter
    if(page_buffer[0] < 1){
        page_buffer[0] = page_buffer[0] + 0.1;    
    }
    else{
        page_buffer[0] = 0;
    }
    */
    
    
    // store the page
    bd.program(page_buffer, 0x0, BD_PAGE_SIZE);
    bd.program(circular_buffer, 0x0, BD_PAGE_SIZE);

    //led = !led;
    //brightness += 0.1; led = (page_buffer[0]%10)/10;
    //led = (page_buffer[0])/10;
    printf("Brightness is now %f\n", *page_buffer);
    
     
     page_counter[0]++;

    // store the page
    bd.program(page_counter, 0x0, BD_PAGE_SIZE);

    //printf("Counter is now %d\n", page_counter[0]);
    
}
void clap(){
    /*
     page_counter[0]++;

    // store the page
    bd.program(page_counter, 0x0, BD_PAGE_SIZE);
*/
    bd.program(i, 0x0, BD_PAGE_SIZE);
    bd.program(circular_buffer, 0x0, BD_PAGE_SIZE);
    bd.program(page_old_counter, 0x0, BD_PAGE_SIZE);
    bd.program(page_buffer, 0x0, BD_PAGE_SIZE);
    if(i[0] < 10){
        circular_buffer[i[0]++] = page_counter[0] - *page_old_counter;    
    }
    else{
        i[0] = 0;
        circular_buffer[i[0]++] = page_counter[0] - *page_old_counter;
    }
    
    page_buffer[0] = max_buffer(circular_buffer);
    printf("Counter is pushed: %u times\n", page_counter[0] - *page_old_counter);
    *page_old_counter = page_counter[0];
    
    led = (page_buffer[0])/10;
    
    bd.program(page_old_counter, 0x0, BD_PAGE_SIZE);
}

uint32_t max_buffer(uint32_t *arr){
    uint32_t max = 0;
    for(uint32_t i = 0; i < 10; i++){
        if(arr[i] > max ){
            max = arr[i];
        }
        if(arr[i] > 0){
            arr[i]--;    
        }
        
    }
    printf("Max is set to: %u \n", max);
    return max;
}

int main() {
    
    circular_buffer = (uint32_t *)malloc(BD_PAGE_SIZE);
    i = (int *)malloc(BD_PAGE_SIZE);
    if (bd.init() != 0) {
        printf("Blockdevice initialization failed!\n");
        return 1;
    }
    //printf("Page Size is: %d", BD_PAGE_SIZE);
    // initialize memory for the page buffer
    page_buffer = (float*)malloc(BD_PAGE_SIZE);
    page_counter = (uint32_t*)malloc(BD_PAGE_SIZE);
    page_old_counter = (uint32_t*)malloc(BD_PAGE_SIZE);

    // read the data back from the block device
    bd.read(page_buffer, 0x0, BD_PAGE_SIZE);
    bd.read(page_counter, 0x0, BD_PAGE_SIZE);
    bd.read(page_old_counter, 0x0, BD_PAGE_SIZE);
    bd.read(circular_buffer, 0x0,BD_PAGE_SIZE );
    bd.read(i, 0x0,BD_PAGE_SIZE );
    
    *page_buffer = 0;
    *page_counter = 0;
    
    printf("Brightness initial value is %f\n", page_buffer[0]);
    printf("Counter initial value is %u\n", page_counter[0]);
    flipper2.attach(&clap,2.0);
    //flipper.attach(&clap,2.0);
    // button fall IRQ handler
    *page_old_counter = page_counter[0];
    btn.fall(queue.event(&btn_fall));
    //btn.fall(queue.event(&clap_count));

    queue.dispatch_forever();
}

