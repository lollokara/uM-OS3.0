/*
 * Author: Lorenzo Karavania <lorenzo@karavania.com>
 * Copyright (c) 2016.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * @ingroup basic
 * @uM-OS the basic OS for uM-OS Project.
 * @ V 0.1 22/05/2016
 * @ V 0.2 27/05/2016
 * @ V 0.3 04/06/2016
 * @ V 0.4 10/06/2016
 * @ V 0.5 18/06/2016
 * @ V 0.6 15/11/2016
 */

#include <iostream>
#include "oled/Edison_OLED.h"
#include "gpio/gpio.h"
#include "math.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib/libconfig.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
//////////////////////////////////////////////////////////////////////////////////////
#define TESTING 		false											//TESTING!!!!/ if (not 1)=0 = 0 if (not 0)1 = 1
//////////////////////////////////////////////////////////////////////////////////////
#define BoxSizex		64												//Menu Size///
#define BoxSizey		11												//Menu Size///
#define MAXPOS			4												//Menu Items//
#define MAXPAGE			1												//Menu Pages//
#define NELEMWIFI		3												//Wifi Eleme//
#define NELEMGEN		2												//Gen  Eleme//
#define MAXRANGE		4												//Max  Range//
//////////////////////////////////////////////////////////////////////////////////////
using namespace std;													//
//
void setupOLED();														// Init Oled & WIFI
void cleanUp();															// Clean Oled
void drawBatt(bool BoxSelected);										// Draw Battery Status
void Menu();															// Main Menu
void DrawMenu(int CursorPos,int Elements);								// Draw Boxes
void DrawPage(int Page, int CursorPos);									// Draw Text (for main and other menus)
void Selected(int Page,int CursorPos);									// Call the function related to the selected page and cursor (NO CODE HERE)
void Info();															// Display Version
void Voltmeter();														// Main purpose
float ReadADC(float PreviousValue);										// Read ADC (demo)
void displayBar(float Value,int Range, bool Auto);						// Display the bar of the current value
char *GetIP();															// ALPHA!!!! Displays IP Addr ( could potentially overwrite code memory, no preallocation done )
bool readConf(int Index);												// Configuration Parser
void WriteConf(int Value1, int Value2);									// Write And Saves The Conf ### Maybe a better approach could be taken for avoiding corruption
void WifiSett();														// Same as Menu (related at the WIFI)
void SetWiFi(int load);													// Call a shell for setting the WIFI
void GenSett();															// Same as Menu (related at the General Settings)
//////////////////////////////////////////////////////////////////////////
const char *Config_dir="/home/root/uM_Conf.cfg";						// CONFIG FILE DO NOT FUCKING TOUCH IT, CAZZO!
//////////////////////////////////////////////////////////////////////////
edOLED oled;															//
gpio BUTTON_UP(47, INPUT);												//
gpio BUTTON_DOWN(44, INPUT);											//
gpio BUTTON_LEFT(165, INPUT);											//
gpio BUTTON_RIGHT(45, INPUT);											//
gpio BUTTON_SELECT(48, INPUT);											//
gpio BUTTON_A(49, INPUT);												//
gpio BUTTON_B(46, INPUT);												//
//////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
    setupOLED();
    Menu();
    
}

void setupOLED()
{
    oled.begin();
    oled.clear(PAGE);
    oled.display();
    oled.setFontType(1);
    oled.setCursor(10,15);
    oled.print("uM-OS");
    oled.setFontType(0);
    for(int i=0;i<101;i=i+5){
        if (i==10 || i==55 || i==65) SetWiFi(i);
        oled.setCursor(24,36);
        oled.print(i);
        oled.setCursor(42,36);
        oled.print("%");
        oled.rect(0,30,64,4);
        float size=i/(100.00/64.00);
        oled.rectFill(0,31,int(size),2);
        oled.display();
    }
    oled.clear(PAGE);
}

void drawBatt(bool BoxSelected)
{
    FILE *fp;
    char Batt[32];
    fp = popen("battery-voltage", "r");
    fgets(Batt, 32, fp);
    fgets(Batt, 32, fp);
    pclose(fp);
    char *p = Batt;
    int val=0;
    while (*p) { // While there are more characters to process...
        if (isdigit(*p)) { // Upon finding a digit, ...
            val = strtol(p, &p, 10); // Read a number, ...
            if(TESTING) printf("%d\n", val); // and print it.
            break;
        } else { // Otherwise, move on to the next character.
            p++;
        }
    }
    if(BoxSelected) oled.setColor(BLACK);
    else oled.setColor(WHITE);
    oled.rect(51,0,12,4);
    oled.rectFill(52,1,(val+1)/10,2);
    oled.rect(63,1,1,2);
    oled.setColor(WHITE);
}

void Menu(){
    int CursorPos=0;
    int Page=0;
    while(0==0){
        DrawMenu(CursorPos,MAXPOS);
        drawBatt(false);
        DrawPage(Page,CursorPos);
        oled.display();
        while (
               (BUTTON_UP.pinRead() == HIGH) &&
               (BUTTON_DOWN.pinRead() == HIGH) &&
               (BUTTON_SELECT.pinRead() == HIGH) &&
               (BUTTON_LEFT.pinRead() == HIGH) &&
               (BUTTON_RIGHT.pinRead() == HIGH) &&
               (BUTTON_A.pinRead() == HIGH) &&
               (BUTTON_B.pinRead() == HIGH)) usleep(100000);
        if (BUTTON_UP.pinRead() == LOW){
            if(CursorPos==0){
                CursorPos=MAXPOS-1;
                if(Page==0){
                    Page=MAXPAGE;
                }
                else Page--;
            }
            else CursorPos--;
        }
        if (BUTTON_DOWN.pinRead() == LOW){
            if(CursorPos==MAXPOS-1){
                CursorPos=0;
                if(Page==MAXPAGE){
                    Page=0;
                }
                else Page++;
            }
            else CursorPos++;
        }
        if (BUTTON_RIGHT.pinRead() == LOW){
            if(Page==MAXPAGE){
                Page=0;
            }
            else Page++;
        }
        
        if (BUTTON_LEFT.pinRead() == LOW){
            
            if(Page==0){
                Page=MAXPAGE;
            }
            else Page--;
            
        }
        if (BUTTON_SELECT.pinRead() == LOW){
            Selected(Page,CursorPos);
        }
        
    }
}

void DrawMenu(int CursorPos,int Elements){
    for(int i=0; i<Elements;i++){
        oled.rect(0,BoxSizey*i+5,BoxSizex,BoxSizey);
        if(CursorPos==i) oled.setColor(WHITE);
        else oled.setColor(BLACK);
        oled.rectFill(0,BoxSizey*i+5,BoxSizex,BoxSizey-1);
        oled.setColor(WHITE);
    }
}

void DrawPage(int Page, int CursorPos){
    const char* Voices[MAXPAGE+4][MAXPOS];
    Voices[0][0]="V";
    Voices[0][1]="OMH";
    Voices[0][2]="mA";
    Voices[0][3]="A";
    Voices[1][0]="ADC  Sett.";
    Voices[1][1]="Gen. Sett.";
    Voices[1][2]="Wifi Sett.";
    Voices[1][3]="Info";
    Voices[2][0]="";
    Voices[2][1]="";
    Voices[2][2]="";
    Voices[2][3]="";
    Voices[3][0]="Power OFF";
    Voices[3][1]="Back";
    Voices[3][2]="";
    Voices[3][3]="";
    Voices[4][0]="WiFi";
    Voices[4][1]="AP";
    Voices[4][2]="Back";
    Voices[4][3]="";
    if(Page==4){
        bool AP=readConf(0);
        bool WiFi=readConf(1);
        if(CursorPos==0) oled.setColor(BLACK);
        else oled.setColor(WHITE);
        oled.setCursor(45,7);
        if(WiFi)oled.print("Ena");
        else oled.print("Dis");
        if(CursorPos==1) oled.setColor(BLACK);
        else oled.setColor(WHITE);
        oled.setCursor(45,7+BoxSizey);
        if(AP)oled.print("Ena");
        else oled.print("Dis");
        oled.setCursor(0,7+BoxSizey*3);
        oled.setColor(WHITE);
        oled.print(GetIP());
    }
    for(int i=0;i<MAXPOS;i++){
        if(CursorPos==i) oled.setColor(BLACK);
        else oled.setColor(WHITE);
        oled.setCursor(1,7+BoxSizey*i);
        if(TESTING) printf("Page=%d  i=%d \n",Page,i);
        oled.print(Voices[Page][i]);
        oled.setColor(WHITE);
    }
    
}

void Selected(int Page,int CursorPos){
    bool AP=0;
    bool WiFi=0;
    if(Page==4){
        AP=readConf(0);
        WiFi=readConf(1);
    }
    switch (Page) {
        case 0:
            switch (CursorPos) {
                case 0:
                    Voltmeter();
                    break;
            }
            break;
        case 1:
            switch (CursorPos) {
                case 0:
                    cleanUp();
                    break;
                case 1:
                    cleanUp();
                    GenSett();
                    break;
                case 2:
                    cleanUp();
                    WifiSett();
                    break;
                case 3:
                    Info();
                    
                    break;
                    
            }
            break;
        case 3:
            switch (CursorPos) {
                case 0:
                    cleanUp();
                    oled.setCursor(12,15);
                    oled.print("Bye Bye");
                    oled.setCursor(26,30);
                    oled.print(";(");
                    oled.display();
                    if(!TESTING) WriteConf(0,0); //Disable For Testing
                    if(!TESTING) system("shutdown -P now");
                    break;
                case 1:
                    Menu();
                    break;
                    
            }
            break;
        case 4:
            switch (CursorPos) {
                case 0:
                    if(WiFi) WriteConf(0,0);
                    else WriteConf(0,1);
                    SetWiFi(999);
                    cleanUp();
                    WifiSett();
                    break;
                case 1:
                    if(AP) WriteConf(0,1);
                    else WriteConf(1,1);
                    SetWiFi(999);
                    cleanUp();
                    WifiSett();
                    break;
                case 2:
                    cleanUp();
                    Menu();
                    break;
            }
            break;
    }
}

void Info(){
    cleanUp();
    oled.setFontType(1);
    oled.setCursor(10,10);
    oled.print("uM-OS");
    oled.setFontType(0);
    oled.setCursor(12,25);
    oled.print("V 0.5B");
    oled.setCursor(52,40);
    oled.print("LK");
    oled.display();
    while ((BUTTON_SELECT.pinRead() == HIGH));
    if (BUTTON_SELECT.pinRead() == LOW){
        Menu();
    }
    
}

void cleanUp()
{
    oled.clear(PAGE);
    oled.display();
}

void Voltmeter(){
    float InitValue=ReadADC(0);
    int Range=0;
    bool Auto=1;
    int RangeMod=0;
    usleep((500)*1000);
    while (true){
        oled.clear(PAGE);
        oled.rect(0,7,80,1);
        float Value=0;
        if(InitValue<2) Range=2;
        else if(InitValue<20) Range=20;
        else if (InitValue<50) Range=50;
        else if (InitValue<100) Range=100;
        if (RangeMod==1) Range=2;
        if (RangeMod==2) Range=20;
        if (RangeMod==3) Range=50;
        if (RangeMod==3) Range=100;
        if (RangeMod==0) Auto=1;
        else Auto=0;
        switch(Range){
            case 2:
                Value=(int(ReadADC(InitValue)*1000))/1000.0000;
                break;
            case 20:
                Value=(int(ReadADC(InitValue)*100))/100.000;
                break;
            case 50:
                Value=(int(ReadADC(InitValue)*100))/100.00;
                break;
            case 100:
                Value=(int(ReadADC(InitValue)*100))/100.00;
                break;
        }
        if(int(Value)<Range){
            displayBar(Value,Range, Auto);
            oled.setFontType(1);
            oled.setCursor(55,15);
            oled.print("V");
            oled.setCursor(5,15);
            oled.print(Value);
            oled.setFontType(0);
        }
        else{
            displayBar(Range,Range, Auto);
            oled.setFontType(1);
            oled.setCursor(25,15);
            oled.print("OL");
            oled.setFontType(0);
        }
        oled.display();
        if (BUTTON_B.pinRead() == LOW){
            oled.clear(PAGE);
            if(RangeMod==0){
                RangeMod=MAXRANGE;
            }
            else RangeMod--;
        }
        if (BUTTON_A.pinRead() == LOW){
            oled.clear(PAGE);
            if(RangeMod==MAXRANGE){
                RangeMod=0;
            }
            else RangeMod++;
        }
        if(BUTTON_UP.pinRead() == LOW) InitValue++;
        if(BUTTON_DOWN.pinRead() == LOW) InitValue--;
        
        if(BUTTON_SELECT.pinRead() == LOW) break;
    }
}

void displayBar(float Value,int Range,bool Auto){
    oled.rect(0,32,64,4);
    float size=Value/(Range/64.000);
    oled.rectFill(0,33,int(size),2);
    oled.setFontType(0);
    oled.setCursor(45,40);
    oled.print(Range);
    if(Auto){
        oled.setCursor(10,40);
        oled.print("Auto");
    }
    
}

float ReadADC(float PreviousValue){
    float Val=0;
    if(PreviousValue==0) Val=(random()/(RAND_MAX/100.0000));
    //if(TESTING) printf("Val= %f  ",Val);
    if(PreviousValue==0) return(Val);
    if(PreviousValue!=0) Val=((1-(random()/(RAND_MAX/2.0000)))/(1+(2.0001-PreviousValue/10.00)));
    return(PreviousValue+Val);
}

char *GetIP(){
    int fd;
    struct ifreq ifr;
    char iface[] = "wlan0";
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    char *Cropped= (inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));
    Cropped+=4;
    return (Cropped);
}

void SetWiFi(int load){
    bool AP=readConf(0);
    bool WiFi=readConf(1);
    if (load==999){
        oled.setCursor(40,7+BoxSizey*2);
        oled.print("Wait");
        oled.display();
    }
    if(AP){
        if(!TESTING) if(load==10 || load ==999) system("systemctl stop wpa_supplicant ");
        if(!TESTING) if(load==55 || load ==999) system("systemctl start hostapd ");
    }
    else if (WiFi && !AP){
        if(!TESTING) if(load==10 || load ==999) system("wpa_cli reconfigure ");
        if(!TESTING) if(load==55 || load ==999) system("systemctl stop hostapd");
        if(!TESTING) if(load==65 || load ==999) system("systemctl start wpa_supplicant ");
    }
    else {
        
        if(!TESTING) if(load==10 || load ==999) system("systemctl stop wpa_supplicant");
        if(!TESTING) if(load==65 || load ==999) system("systemctl stop hostapd ");
    }
}

bool readConf(int Index){
    config_t cfg;
    config_setting_t *setting;
    const char *str;
    config_init(&cfg);
    config_read_file(&cfg, Config_dir);
    if(config_lookup_string(&cfg, "name", &str)) printf("CFG name: %s\n\n", str);
    setting = config_lookup(&cfg, "wifi_sett.AP");
    config_setting_t *AP = config_setting_get_elem(setting, 0);
    int Active;
    if(Index==0) config_setting_lookup_int(AP, "status", &Active);
    else config_setting_lookup_int(AP, "wifi", &Active);
    return(Active);
}

void WriteConf(int Value1, int Value2){
    config_t cfg;
    config_setting_t *setting,*root;
    config_init(&cfg);
    config_read_file(&cfg, Config_dir);
    root = config_root_setting(&cfg);
    setting = config_setting_get_member(root, "wifi_sett");
    setting = config_setting_get_member(setting, "AP");
    config_setting_t *AP = config_setting_get_elem(setting, 0);
    AP = config_setting_lookup(AP,"status");
    config_setting_set_int(AP, Value1);
    AP = config_setting_get_elem(setting, 0);
    AP = config_setting_lookup(AP,"wifi");
    config_setting_set_int(AP, Value2);
    config_write_file(&cfg, Config_dir);
    config_destroy(&cfg);
}

void WifiSett(){
    int CursorPos=0;
    int Page=4;
    while(0==0){
        DrawMenu(CursorPos, NELEMWIFI);
        drawBatt(false);
        DrawPage(Page,CursorPos);
        oled.display();
        while ((BUTTON_UP.pinRead() == HIGH) && (BUTTON_DOWN.pinRead() == HIGH) && (BUTTON_SELECT.pinRead() == HIGH) && (BUTTON_LEFT.pinRead() == HIGH)) usleep(100000);
        if (BUTTON_LEFT.pinRead() == LOW){
            Menu();
        }
        if (BUTTON_UP.pinRead() == LOW){
            if(CursorPos==0){
                CursorPos=NELEMWIFI-1;
            }
            else CursorPos--;
        }
        if (BUTTON_DOWN.pinRead() == LOW){
            if(CursorPos==NELEMWIFI-1){
                CursorPos=0;
            }
            else CursorPos++;
        }
        if (BUTTON_SELECT.pinRead() == LOW){
            Selected(Page,CursorPos);
        }
    }
}

void GenSett(){
    int CursorPos=0;
    int Page=3;
    while(0==0){
        DrawMenu(CursorPos, NELEMGEN);
        drawBatt(false);
        DrawPage(Page,CursorPos);
        oled.display();
        while ((BUTTON_UP.pinRead() == HIGH) && (BUTTON_DOWN.pinRead() == HIGH) && (BUTTON_SELECT.pinRead() == HIGH) && (BUTTON_LEFT.pinRead() == HIGH)) usleep(100000);
        if (BUTTON_LEFT.pinRead() == LOW){
            Menu();
        }
        if (BUTTON_UP.pinRead() == LOW){
            if(CursorPos==0){
                CursorPos=NELEMGEN-1;
            }
            else CursorPos--;
        }
        if (BUTTON_DOWN.pinRead() == LOW){
            if(CursorPos==NELEMGEN-1){
                CursorPos=0;
            }
            else CursorPos++;
        }
        if (BUTTON_SELECT.pinRead() == LOW){
            Selected(Page,CursorPos);
        }
    }
}
