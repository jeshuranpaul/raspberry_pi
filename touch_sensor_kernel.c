#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
#include "io.h"

enum {
	// GPIO registers
	GPLEV = 0x7e200034
};

enum {
	//pins
	OUT_GPIO_PIN  = 13,
	LED_GPIO_PIN = 24
};

enum {
	//commands
	CMDstart,
	CMDstop,
	CMDon,
	CMDoff,
	CMDblink,
	CMDhelp
};

static Cmdtab ledcmd[] = {
	{CMDstart, "start", 1},
	{CMDstop, "stop", 1},
	{CMDon, "on", 1},
	{CMDoff, "off", 1},
	{CMDblink, "blink", 1},
	{CMDhelp, "help", 1}
};

static int state = 0, touch_count = 0;
static long ledread(Chan*, void*, long, vlong);
static long ledwrite(Chan*, void*, long, vlong);
static int checkOnOff(int);
static void kprocHandler(void *);

static int funcs[] = {Output};

void ledlink(void) {
	addarchfile("led", 0666, ledread, ledwrite);
}

static long ledread(Chan*, void*, long, vlong) {
	char buff[35];
	switch(state) {
	case 0:
		strcpy(buff, "Mode: off \t Sensing: off \n");
		break;

	case 1: 	//sensing on
		switch(touch_count % 3) {
		case 0:
			strcpy(buff, "Mode: off \t ");
			break;
		case 1: 
			strcpy(buff, "Mode: on \t ");
			break;
		case 2:
			strcpy(buff, "Mode: blink \t ");
			break;
		}	
		strcat(buff, "Sensing: on \n");
		break;

	case 2: 	//on mode
		strcpy(buff, "Mode: on \t Sensing: off \n");
		break;

	case 3:	//off mode
		strcpy(buff, "Mode: off \t Sensing: off \n");
		break;

	case 4: 	//blink mode
		strcpy(buff, "Mode: blink \t Sensing: off \n");
		break;
	}
	
	//return readstr(0, lbuf, n, buff);
	print("%s\n", buff);
	return 0;
}

static long ledwrite(Chan *, void *buff, long n, vlong) {
	//similar to gpiowrite

	Cmdbuf *cb;
	Cmdtab *ct;
	
	cb = parsecmd(buff, n);
	if(waserror()) {
		free(cb);
		nexterror();
	}
	
	ct = lookupcmd(cb, ledcmd, nelem(ledcmd));
	
	gpiosel(LED_GPIO_PIN, Output);

	switch(ct->index) {
	case CMDstart:
		print("start -> Touch sensor to change modes \n");
		state = 1;
		gpioout(LED_GPIO_PIN, 0);
		kproc("led", kprocHandler, nil);
		break;
	
	case CMDstop:
		print("stop -> Touch sensing has been stopped now\n");
		state = 3;
		touch_count = 0;
		gpioout(LED_GPIO_PIN, 0);
		break;

	case CMDon:
		print("on -> LED switched on \n");
		state = 2;
		touch_count = 0;
		gpioout(LED_GPIO_PIN, 1);
		break;
	
	case CMDoff:
		print("off -> LED switched off \n");
		state = 3;
		touch_count = 0;
		gpioout(LED_GPIO_PIN, 0);
		break;

	case CMDblink:
		print("blink -> LED is blinking \n");
		state = 4;
		touch_count = 0;
		kproc("led", kprocHandler, nil);
		break;
	
	case CMDhelp: 
		print("Valid commands are: \n start \t stop \t on \t off \t blink \n");
		print("Usage: \n echo [command] > /dev/led \n");
	}
	free(cb);
	poperror();
	return n;
}

static int checkOnOff(int doprint) {
	char buf1[16], buf2[16];
	char *e, *f;
	e = buf1 + sizeof(buf1);
	f = buf2 + sizeof(buf2);

	seprint(buf1, e, "%08ulx", ((ulong *)GPLEV)[0]);
	seprint(buf2, f, "%08ulx", ((ulong *)GPLEV)[1]);
	
	if(doprint == 1) {
		print("Reg vals: %s%s \n", buf2, buf1);
	}
	vlong value, out_val;
	
	if(OUT_GPIO_PIN < 31) 		value = strtoull(buf1, nil, 16);
	else 						value = strtoull(buf2, nil, 16);

 	out_val = value & (1 << OUT_GPIO_PIN);
	
	if(out_val == 0) 	return 0;
	else				return 1;
}

static void kprocHandler(void *) {
	int prev_state, curr_state;

	prev_state = checkOnOff(0);

	if(state == 1) {
		while(state == 1) {
			curr_state = checkOnOff(0);

			if(prev_state != curr_state) {
				touch_count++;
				prev_state = curr_state;
				checkOnOff(1);	//for printing reg values
			}
			
			switch(touch_count % 3) {
			case 0:
				gpioout(LED_GPIO_PIN, 0);
				break;
		
			case 1:
				gpioout(LED_GPIO_PIN, 1);
				break;

			case 2: 
				gpioout(LED_GPIO_PIN, 1);
				tsleep(&up->sleep, return0, 0, 250);
				gpioout(LED_GPIO_PIN, 0);
				tsleep(&up->sleep, return0, 0, 250);
				break;
			}
		}
		//had to do this, because state could change between sleeps
		if(state == 2) gpioout(LED_GPIO_PIN, 1);
	}
	
	else if(state == 4) {
		while(state == 4) {
			gpioout(LED_GPIO_PIN, 1);
			tsleep(&up->sleep, return0, 0, 250);
			gpioout(LED_GPIO_PIN, 0);
			tsleep(&up->sleep, return0, 0, 250);
		}
		
		//had to do this, because state could change between sleeps
		if(state == 2) gpioout(LED_GPIO_PIN, 1);
	}
}
