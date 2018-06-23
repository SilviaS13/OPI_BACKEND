#include <wiringPi.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
//#include <zconf.h>
#include "transmit.h"

#define NUM_OF_STRINGS          24
#define STRING_LEN              100
#define COMMAND_SIZE_REZERV     100
#define CLOCK                   10
#define LIGHT                   7
#define MAX_PROP_LEN            30
#define DEMO_SECONDS            15
#define MODE_OFF                5
#define DEBUG                   0


enum{HRS, MINS, MODE_C, RC, GC, BC, MUSIC, MUS_E, DEMO_C, ENABLED_C};
enum{NAME, MODE_L,RL, GL, BL, DEMO_L, ENABLED_L};
//                      000         001         010        011      100
char *availModes[] = {"Веселка", "Спалахи", "Світанок", "Колесо", "Колір"};
char *availMusic[] = {"Пташки у лісі", "Журчання води", "Вітер в полі", "Шум дощу", "Мелодія світанку", "Затишна домівка"};

struct Clock{
    char clockProperties[CLOCK][MAX_PROP_LEN];
    char lightProperties[LIGHT][MAX_PROP_LEN];
    char enabled_light[MAX_PROP_LEN];
    int last_enabled_hrs ;
    int last_enabled_mins;
} properties;

struct Commands{
    char getClocks[COMMAND_SIZE_REZERV];
    char getLights[COMMAND_SIZE_REZERV];
    char cat[10];
    const char grep[20];
    char sed[20];
    char sedReplace;
    char sedDelLines;
    char fullCommand[COMMAND_SIZE_REZERV];
}commands;

struct Files{
    int propCounter;
    int currentProp;
    char clocks[NUM_OF_STRINGS][STRING_LEN];
    char lights[NUM_OF_STRINGS][STRING_LEN];
    FILE * fp;
    char returned_property_string[STRING_LEN];
}file;

void initCommands(){
    sprintf(commands.getClocks,"/root/conf/clocks"/*"/home/silvia/CLionProjects/conf/clocks"*/);
    sprintf(commands.getLights, "/root/conf/lights"/*"/home/silvia/CLionProjects/conf/lights"*/);
    strcpy(commands.cat, "/bin/cat");
    strcpy(commands.grep, "/bin/grep");
    strcpy(commands.sed, "/bin/sed -i");
    commands.sedReplace= 'g';
    commands.sedDelLines='d';
    file.propCounter = 0;
    strcpy(properties.enabled_light, "none");
    properties.last_enabled_hrs = 24;
    properties.last_enabled_mins = 60;
}

//ONE_WIRE interface

//11 10 xxx xxxx_xxxx xxxx_xxxx xxxx_xxxx 11 - write
//11 01 xxx xxxx_xxxx xxxx_xxxx xxxx_xxxx 11 - read

//int sda_pulse_width = 40000;//us

char * getProperty(int type, int prop_c, int prop_l){
    return (type == CLOCK ? properties.clockProperties[prop_c] : properties.lightProperties[prop_l]);
}

void prepareColorModeAndSend(int type){
    int i;
    int mode = MODE_OFF;
    char * mode_s = getProperty(type, MODE_C, MODE_L);
    for (i = 0; i< (sizeof(availModes)/sizeof(availModes[0])); i++){
        if(strcmp(mode_s, availModes[i]) == 0){
            mode = i;
            break;
        }
    }
    int r = atoi(getProperty(type, RC, RL));
    int g = atoi(getProperty(type, GC, GL));
    int b = atoi(getProperty(type, BC, BL));
    sendNeopixel(mode, r, g, b);
}

void readButtonAndWaitForTurningOff(){
    unsigned long long time_counter;
    int on, off;
    char enabled = 1;
    on = 0;
    off = 0;
    while (digitalRead (BTN) == 1){
        usleep(10000);//sleep 100ms
#if DEBUG ==2
        printf("TURN ON MUSIC IF AVAILABLE");
#endif
        if (strcmp(properties.clockProperties[MUS_E], "t") == 0){
#if DEBUG==2
            printf("TURN ON MUSIC %s \n dont forget to write this fcn\n", properties.clockProperties[MUSIC]);
#endif
            if (enabled == 1){
                if (on == 0) {
                    softToneWrite(BUZZ, BUZZ_FREQ);
#if DEBUG
                    printf("ON\n");
#endif
                }
                on++;
                if (on == BUZZ_TIME_ON){
                    enabled = 0, off = 0;
                }
            }
            else{
                if (off == 0){
                    softToneWrite(BUZZ, 0);
#if DEBUG
                    printf("OFF\n");
#endif
                }
                off++;
                if (off == BUZZ_TIME_OFF){
                    enabled = 1, on = 0;
                }
            }
        }
        if (time_counter == 300000) {//5 mins
            softToneWrite(BUZZ, 0);
#if DEBUG
            printf("OFF");
#endif
            break;
        }
    }
    if (strcmp(properties.clockProperties[MUS_E], "t") == 0) {
        softToneWrite(BUZZ, 0);
#if DEBUG
        printf("OFF");
#endif
    }
    sendNeopixel(MODE_OFF, 0, 0, 0);
}

void writeCommand(int type){
    char old[STRING_LEN];

    sprintf(old, type== CLOCK? file.clocks[file.currentProp]: file.lights[file.currentProp]);

    int i,j;
    i = file.currentProp;

    if (type==CLOCK){
        j = (int)strlen(file.clocks[file.currentProp]) - 3;
        file.clocks[i][j] = 'f';

    } else{
        j = (int)strlen(file.lights[file.currentProp]) - 3;
        file.lights[i][j] = 'f';
    }
    sprintf(commands.fullCommand, "%s 's/%s/%s/g' %s\n", commands.sed, old,
            type== CLOCK? file.clocks[file.currentProp]: file.lights[file.currentProp],
            type== CLOCK? commands.getClocks : commands.getLights
    );

#if DEBUG
    printf("WRITE FCN, %s",commands.fullCommand);
#endif

    file.fp = popen(commands.fullCommand, "r");
    pclose(file.fp);
}

void executeCommand(int type){
    sprintf(commands.fullCommand, "%s %s\n", commands.cat, type==CLOCK? commands.getClocks : commands.getLights);
#if DEBUG
    printf(commands.fullCommand);
#endif
    file.fp = popen(commands.fullCommand, "r");
    if (file.fp == NULL) {
        printf("can`t open file: %s \n" , commands.fullCommand);
        return;
    }
    else{
        file.propCounter = 0;
        int m;

        for (m = 0; m <NUM_OF_STRINGS; m++) {
            strcpy(type == CLOCK ? file.clocks[m] : file.lights[m], "");
        }

        while (fgets(file.returned_property_string,
                     sizeof(file.returned_property_string)-1, file.fp))
        {
            size_t len_of_prop_row =  strlen(file.returned_property_string);
            if (len_of_prop_row > STRING_LEN) {
                printf("Wrong property row value length for %s %ld\n",
                       file.returned_property_string);
                return;
            }
            else {
                char tmp[STRING_LEN];
                int j;
                //i = file.propCounter;
                j = (int)strlen(file.returned_property_string)-1;

                if (type==CLOCK){
                    file.returned_property_string[j] = '\0';
                } else{
                    file.returned_property_string[j] = '\0';
                }

                strcpy(type == CLOCK ? file.clocks[file.propCounter] : file.lights[file.propCounter], file.returned_property_string);

#if DEBUG == 2
                printf("String property: %s\n", type == CLOCK ? file.clocks[file.propCounter] : file.lights[file.propCounter]);
#endif
                file.propCounter++;
            }
        }
    }
    pclose(file.fp);
    return;
}

void parseProperties(int type, int i){
    char str[STRING_LEN]; //temp storage for one property string
    int j, k;

    for (j = 0; j < type; j++){
        strcpy(type == CLOCK ? properties.clockProperties[j] : properties.lightProperties[j], "");
    }

    int count_of_elems = 0;
    sprintf(str, type == CLOCK ? file.clocks[i]: file.lights[i]);
    //count of commas/elems - 1
    for (j = 0; j < strlen(str); j++){
        count_of_elems += (str[j] == ',') ? 1 : 0;
    }

    k=0;
    // checking of true count
    if ((count_of_elems+1) == type){
        for (j = 0; j <= strlen(str); j++) {
            if (str[j] != ',') {
                char str_c[2];
                sprintf(str_c, "%c\0", str[j]);
                strcat(type == CLOCK ? properties.clockProperties[k] : properties.lightProperties[k],
                       str_c);
            } else{
#if DEBUG == 2
                printf("Single prop: %s \n",type == CLOCK ? properties.clockProperties[k] : properties.lightProperties[k]);
#endif
                k++;
            }
        }
    }
}

void checkProperties(int type){
    int i;
    //works
    if (type == LIGHT && (strcmp(properties.enabled_light, getProperty(type,NAME, NAME)) == 0)){

        if (strcmp(getProperty(type,ENABLED_C, ENABLED_L), "t") == 0)
            return;

        strcpy(properties.enabled_light, "none");
#if DEBUG
        printf("turned off %s\n", getProperty(type,NAME, NAME));
#endif
        sendNeopixel(MODE_OFF, 0, 0, 0);
        return;
    }

    if (strcmp(getProperty(type, DEMO_C, DEMO_L),"t") == 0) {
#if DEBUG
        printf("demo on for %s %s\n", getProperty(type, HRS, NAME), getProperty(type,MINS, MODE_L));
#endif
        prepareColorModeAndSend(type);
        sleep(DEMO_SECONDS);
        writeCommand(type);
        sendNeopixel(MODE_OFF, 0, 0, 0);
    }

        // перевірка на ввімкненість
    if (strcmp(getProperty(type, ENABLED_C, ENABLED_L), "t") == 0 )
    {
        if (type == CLOCK ){
            __time_t raw_time;
            struct tm *time_now;
            time(&raw_time);
            time_now = localtime(&raw_time);
	    
            //the time is matched
            if (time_now->tm_hour == atoi(properties.clockProperties[HRS]) &&
                time_now->tm_min == atoi(properties.clockProperties[MINS]) &&
                time_now->tm_hour != properties.last_enabled_hrs &&
                time_now->tm_min != properties.last_enabled_mins)
            {
#if DEBUG
		printf("enabled clocks case\n");	
#endif
                properties.last_enabled_hrs = (int)time_now->tm_hour;
                properties.last_enabled_mins = (int)time_now->tm_min;

        	prepareColorModeAndSend(type);
#if DEBUG
                printf("CLOCK ON on for %s %s\n", getProperty(type, HRS, HRS), getProperty(type, MINS, MINS ));
#endif
                readButtonAndWaitForTurningOff();
            }
            //reset last time
            if (time_now->tm_min > properties.last_enabled_mins){
                properties.last_enabled_mins = 60;
                properties.last_enabled_hrs = 24;
            }
        }
        else {
            strcpy(properties.enabled_light, getProperty(type, NAME, NAME));
            prepareColorModeAndSend(type);
        }
        return;
    }
}

void readProperties(int type){
    //make command "read all clocks params"
    executeCommand(type);
    int i;
    for (i = 0; i < file.propCounter; i++) {// iteration of all received strings
        file.currentProp = i;
        parseProperties(type, i);
        checkProperties(type);
    }
}

int main(void){
    setup();
    initCommands();
    while (1){
        readProperties(CLOCK);
        readProperties(LIGHT);
        sleep(1);
    }
}
