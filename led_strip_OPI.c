#include <wiringPi.h>
#include <stdio.h>

#define SDA 12
#define LED 30

//mode for write
//111(2) = 7(10) - rainbow
//000 = 0 - single color
//010(2) = 2(10) - start changing
//101(2) = 5(10) - stop changing

//ONE_WIRE interface

//11 10 xxx xxxx_xxxx xxxx_xxxx xxxx_xxxx 11 - write
//11 01 xxx xxxx_xxxx xxxx_xxxx xxxx_xxxx 11 - read

unsigned char buffer[39];
unsigned char light_mode;

unsigned char r = 0;
unsigned char g = 0;
unsigned char b = 0;
unsigned char m = 0;
unsigned char p = 0;

int sda_pulse_width = 40000;//us

void sendBit(char bit){
    
    if (bit == 1){
        digitalWrite(SDA, 1);
	digitalWrite(LED, 1);

        usleep(sda_pulse_width);
    }
    else{

        usleep(sda_pulse_width);
    }

    digitalWrite(SDA, 0);
    digitalWrite(LED, 0);
}

void sendByte(unsigned char byte, int lenght){
    
    char cur_bit = lenght - 1;
    unsigned char bit;
    for( bit = 0 ; bit < lenght ; bit++ ) {
      sendBit( (byte >> cur_bit) & 1 );
      cur_bit--; // and then shift left so bit 6 moves into 7, 5 moves into 6, etc 
    }
}

void startStop(){    
    sendBit(1);
    sendBit(1);
}

void sendNeopixel( unsigned char mode, unsigned char r,unsigned char g, unsigned char b){
    

    startStop();

    sendByte(mode, 3);

    sendByte(r, 8);
    sendByte(g, 8);
    sendByte(b, 8);

    startStop();
    //pinMode(SDA, INPUT);
}

void sendMessage(unsigned char byte, int lenght){
    //pinMode(SDA, OUTPUT);
    startStop();
    sendByte(byte, lenght);
    startStop();
    //pinMode(SDA, INPUT);
}

void setup(){
    wiringPiSetup();
    pinMode(SDA, OUTPUT);
    //pinMode(LED, OUTPUT);
    //digitalWrite(LED, 1);
    //printf("setup complete\n");
}

int main(void){
    
    setup();

    //sendNeopixel(5, 0,0,0);

//printf("next");
//    sleep(11);

//    sendNeopixel(2, 0,0,0);
//printf("next");
//    sleep(11);

//    sendNeopixel( 1, 150, 150, 0);


unsigned int mode = 0;
    while (1){

    printf("mode=");
    scanf("%d", &mode);

if (mode == 1){
    printf("\nr=");
    scanf("%d", &r);
    printf("\ng=");
    scanf("%d", &g);
    printf("\nb=");
    scanf("%d", &b);
    
}
    printf("Mode = %d R= %d G=%d B=%d \n", mode, r , g, b);
    printf("\n");    
    sendNeopixel(mode, r,g,b);
    sleep (5);
    }
}
