//   Smartmeter, Optical power consumption monitor (Raspberry Pi) 
//
//   Hardware:
//      - optical sensor LPT80A for detecting orange led (~600nm) pulse
//         at Imp_GPIO input (default 17) 
//      - 4 digit TM1637 display
//   need library/packages: 
//         https://github.com/GrazerComputerClub/TM1637Display
//         wiringPi
//   compile:  g++ -Wall -o pipomon pipomon.c TM1637Display.cpp -lwiringPi

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include "TM1637Display.h"


static sig_atomic_t end = 0;
const int Imp_GPIO    = 17;   // Impuls Input
const int Imp_per_kWh = 1000; // power meter led output spec. 


volatile int nCounterValue = 0;

//Interrupt Service Routine for GPIO - counter function
void ISR_Imp_GPIO(void) {
	nCounterValue++;
}


static void sighandler(int signo){
	end = 1;
}


int main(void) {
	struct sigaction sa;
	int nCounterValueOld;
	struct timeval time_start, time_end;
	char sz7seg[4+1];
	int nlen, loop;
	struct tm *tm;

	wiringPiSetupGpio();

	if (wiringPiSetupGpio() == -1) {
		printf("wiringPiSetup failed\n\n");
		exit(1) ;
	}
	
	TM1637Display display(23, 24, 40);  // CLK to GPIO 23 and DIO to GPIO 24, 40% brightness

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = sighandler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT,&sa, NULL);
	sigaction(SIGTERM,&sa, NULL);

	nCounterValue = 0;
	nCounterValueOld = 0;
	gettimeofday(&time_start, NULL);

// INT_EDGE_BOTH, INT_EDGE_FALLING, INT_EDGE_RISING only one ISR per input
	printf("activating counter input GPIO %d\n", Imp_GPIO);
//	if (wiringPiISR(Imp_GPIO, INT_EDGE_RISING, &ISR_Imp_GPIO)  < 0) {
	if (wiringPiISR(Imp_GPIO, INT_EDGE_FALLING, &ISR_Imp_GPIO)  < 0) {
		printf("Unable to setup ISR for GPIO %d (%s)\n\n", 
		 Imp_GPIO, strerror(errno));
		exit(1);
	}
	display.Show("init");

	while(!end) {
		if (nCounterValue != nCounterValueOld) {
			gettimeofday(&time_end, NULL);
			tm = localtime(&time_end.tv_sec);
			
			if (nCounterValue>=2) { //start show values after second impuls 
				float fSec, work, power;
				
				fSec = (time_end.tv_sec - time_start.tv_sec)+(time_end.tv_usec-time_start.tv_usec)/1000000.0;
				work = (1.0 * nCounterValue) / Imp_per_kWh;
				power =  1000.0 / Imp_per_kWh *  3600.0 / fSec;
//				printf("counter=%06d, interval=%.1f sec, work=%.3f kWh, power=%5.0f W, date=%02d.%02d.%d, time=%02d:%02d:%02d\n", 
//				nCounterValue, fSec, work, power, tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
				
				printf(" %06d\t%3.1f\t%3.3f\t%5.0f\t%02d.%02d.%d\t%02d:%02d:%02d\n", 
				 nCounterValue, fSec, work, power, tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900, tm->tm_hour, tm->tm_min, tm->tm_sec);		
				
				if (power<=9999) {
					snprintf(sz7seg, 5, "%04.0f", power);
				} else {
					strcpy(sz7seg, " OF "); //overflow
				}
				display.Show(sz7seg);
			} else { //show headerafter first impuls
				printf("counter\tinterval (sec)\twork (kWh)\tpower (W)\tdate (DD:MM:YYYY)\ttime (HH:MM:SS)\n");
			}
			fflush(stdout);
			time_start = time_end;
			nCounterValueOld = nCounterValue;
		}
		delay(100);
	}

	display.Show("off");
	return 0;
}

