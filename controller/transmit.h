#ifndef ALARM_CONTROLLER_TRANSMIT_H
#define ALARM_CONTROLLER_TRANSMIT_H

#include <stdio.h>
#include <softTone.h>

#define SDA                 12
#define LED                 30
#define BTN                 10
#define SDA_PULSE_WIDTH     40000
#define BUZZ 		    11
#define BUZZ_FREQ           900
#define BUZZ_TIME_ON        50
#define BUZZ_TIME_OFF       400

enum {RAINBOW, CHASE, SUNRISE, WHEEL, COLOR, OFF};


// З А Г Л У Ш К А !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/*
void digitalWrite(int pin, short val){
    printf("%d", val);
}

int digitalRead(int pin){
    printf("DIGITAL READ FCN of %d\n", pin);
    return 1;
}
 */
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void sendBit(char bit){

    if (bit == 1){
        digitalWrite(SDA, 1);
        usleep(SDA_PULSE_WIDTH);
    }
    else{

        usleep(SDA_PULSE_WIDTH);
    }

    digitalWrite(SDA, 0);
    digitalWrite(LED, 0);
}

void sendByte(unsigned char byte, int length){

    char cur_bit = length - 1;
    unsigned char bit;
    for( bit = 0 ; bit < length ; bit++ ) {
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
    if (mode == COLOR) {

        sendByte(r, 8);
        sendByte(g, 8);
        sendByte(b, 8);
    }

    startStop();
}
//
//void sendMessage(unsigned char byte, int length){
//    startStop();
//    sendByte(byte, length);
//    startStop();
//}

void setup(){
//    printf("UNCOMMENT THIS --> wiringPiSetup();\n");
//    printf("UNCOMMENT THIS --> pinMode(SDA, OUTPUT);\n");
//    printf("UNCOMMENT THIS --> pinMode(BTN, INPUT);\n");
    wiringPiSetup();
    softToneCreate(BUZZ);
    pinMode(SDA, OUTPUT);
    pinMode(BTN, INPUT);
    pullUpDnControl (BTN, PUD_UP);
}
#endif //ALARM_CONTROLLER_TRANSMIT_H