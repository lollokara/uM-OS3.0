

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

#define TESTING 		1

#define BoxSizex		64
#define BoxSizey		11
#define MAXPOS			4
#define MAXPAGE			1
#define NELEMWIFI		3
#define NELEMGEN		2

using namespace std;

void setupOLED();
void cleanUp();
void drawBatt(bool BoxSelected);
void Menu();
void DrawMenu(int CursorPos,int Elements);
void DrawPage(int Page, int CursorPos);
void Selected(int Page,int CursorPos);
void Info();
void Voltmeter();
float ReadADC(float PreviousValue);
void displayBar(float Value,int Range);
char *GetIP();
bool readConf(int Index);
void WifiSett();
void SetWiFi(int load);
void GenSett();

const char *Config_dir="/home/root/uM_Conf.cfg";

edOLED oled;
gpio BUTTON_UP(47, INPUT);
gpio BUTTON_DOWN(44, INPUT);
gpio BUTTON_LEFT(165, INPUT);
gpio BUTTON_RIGHT(45, INPUT);
gpio BUTTON_SELECT(48, INPUT);
gpio BUTTON_A(49, INPUT);
gpio BUTTON_B(46, INPUT);

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
	        printf("%d\n", val); // and print it.
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
		while ((BUTTON_UP.pinRead() == HIGH) && (BUTTON_DOWN.pinRead() == HIGH) && (BUTTON_SELECT.pinRead() == HIGH) && (BUTTON_LEFT.pinRead() == HIGH) && (BUTTON_RIGHT.pinRead() == HIGH)) usleep(100000);
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
		printf("%d  %d",Page,i);
		oled.print(Voices[Page][i]);
		oled.setColor(WHITE);
	}

}

void Selected(int Page,int CursorPos){
	bool AP=readConf(0);
	bool WiFi=readConf(1);
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
		case 1:
			cleanUp();
			oled.setCursor(12,15);
			oled.print("Bye Bye");
			oled.setCursor(26,30);
			oled.print(";(");
			oled.display();
			if(!TESTING) WriteConf(0,0); //Disable For Testing
			system("shutdown -P now");
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
			break;
		case 3:
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
	oled.print("V 0.2A");
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
	oled.clear(PAGE);
	float InitValue=ReadADC(0);
	int Range=0;
	usleep((500)*1000);
	while (true){
		float Value=0;
		if(InitValue<1) Range=1;
		else if(InitValue<10) Range=10;
		else if (InitValue<20) Range=20;
		switch(Range){
		case 1:
			Value=(int(ReadADC(InitValue)*1000))/1000.0000;
			break;
		case 10:
			Value=(int(ReadADC(InitValue)*100))/100.000;
			break;
		case 20:
			Value=(int(ReadADC(InitValue)*100))/100.00;
			break;
		}
		displayBar(Value,Range);
		oled.setFontType(1);
		oled.setCursor(55,15);
		oled.print("V");
		oled.setCursor(5,15);
		oled.print(Value);
		oled.setFontType(0);

		oled.display();
		if(BUTTON_SELECT.pinRead() == LOW) break;
	}
}

void displayBar(float Value,int Range){
	oled.rect(0,34,64,4);
	float size=Value/(Range/64.000);
	oled.rectFill(0,35,int(size),2);
	oled.setFontType(0);
	oled.setCursor(50,40);
	oled.print(Range);

}

float ReadADC(float PreviousValue){
	float Val=0;
	if(PreviousValue==0) Val=(random()/(RAND_MAX/20.0000));
	printf("Val= %f  ",Val);
	if(PreviousValue==0) return(Val);
	if(PreviousValue!=0) Val=((1-(random()/(RAND_MAX/2.0000)))/(1+(2.0001-PreviousValue/10.00)));///((101.0000-PreviousValue)*100.000
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
		if(!TESTING) if(load==10) system("service stop wpa_supplicant ");
		if(!TESTING) if(load==55) system("service start hostapd ");
	}
	else if (WiFi && !AP){
		if(!TESTING) if(load==10) system("wpa_cli reconfigure ");
		if(!TESTING) if(load==55) system("service stop hostapd");
		if(!TESTING) if(load==65) system("service start wpa_supplicant ");
	}
	else {

		if(!TESTING) if(load==10) system("service stop wpa_supplicant");
		if(!TESTING) if(load==65) system("service stop hostapd ");
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
		while ((BUTTON_UP.pinRead() == HIGH) && (BUTTON_DOWN.pinRead() == HIGH) && (BUTTON_SELECT.pinRead() == HIGH)) usleep(100000);
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

//shutdown -P now

void GenSett(){
	int CursorPos=0;
	int Page=3;
	while(0==0){
		DrawMenu(CursorPos, NELEMGEN);
		drawBatt(false);
		DrawPage(Page,CursorPos);
		oled.display();
		while ((BUTTON_UP.pinRead() == HIGH) && (BUTTON_DOWN.pinRead() == HIGH) && (BUTTON_SELECT.pinRead() == HIGH)) usleep(100000);
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