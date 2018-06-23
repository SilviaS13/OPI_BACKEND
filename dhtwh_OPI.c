#include <wiringPi.h>
#include <lcd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

//USE WIRINGPI PIN NUMBERS
#define LCD_RS  8               //Register select pin
#define LCD_E   9               //Enable Pin
#define LCD_D4  7               //Data pin 4
#define LCD_D5  0               //Data pin 5
#define LCD_D6  2               //Data pin 6
#define LCD_D7  3               //Data pin 7
#define MAXTIMINGS 85
#define DHTPIN 16

int lcd;
int dht11_dat[5] = {0};
time_t rawtime;
struct tm * timeinfo;
int humidity = 0;
int temperature = 0;

void read_dht11_dat()
{
        uint8_t laststate = HIGH;
        uint8_t counter = 0;
        uint8_t j = 0, i;
        float f; 

        dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;

        pinMode(DHTPIN, OUTPUT);
        digitalWrite(DHTPIN, LOW);
        delay(18);
        
        digitalWrite(DHTPIN, HIGH);
        delayMicroseconds(40);
        
        pinMode(DHTPIN, INPUT);

        for (i = 0; i < MAXTIMINGS; i++)
        {
                counter = 0;
                while (digitalRead(DHTPIN) == laststate)
                {
                        counter++;
                        delayMicroseconds(1);
                        if (counter == 255)
                        {
                                break;
                        }
                }
                laststate = digitalRead(DHTPIN);

                if (counter == 255)
                        break;

                if ((i >= 4) && (i % 2 == 0))
                {
                        dht11_dat[j / 8] <<= 1;
                        if (counter > 16)
                                dht11_dat[j / 8] |= 1;
                        j++;
                }
         }

        if ((j >= 40) && (dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF)))
        {
	    temperature  = dht11_dat[2];/* *9. / 5. + 32; for fahrenheit*/
	    humidity = dht11_dat[0];
	    //printf("heeeey!");
	}
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
                lcdClear(lcd);
		lcdPosition(lcd, 0, 0);
		printf("%s", timeinfo);
		lcdPrintf(lcd, asctime(timeinfo));

                lcdPosition(lcd, 0, 1);
                lcdPrintf(lcd, "Temp: %dC ", temperature);

		lcdPosition(lcd, 10, 1);
                lcdPrintf(lcd, "H: %d%%", humidity);
        
}

int main(void)
{
        int lcd;
        wiringPiSetup();
        lcd = lcdInit (2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);
        
        while (1)
        {
                read_dht11_dat();
                sleep(10);
        }

        return(0);
}