#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"
#include "io.h"

#define GPIOREGS (VIRTIO+0x200000)

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
	CMDblink
};

static Cmdtab ledcmd[] = {
	{CMDstart, "start", 1},
	{CMDstop, "stop", 1},
	{CMDon, "on", 1},
	{CMDoff, "off", 1},
	{CMDblink, "blink", 1}
};

int state = 0;
static long ledread(Chan*, void*, long, vlong);
static long ledwrite(Chan*, void*, long, vlong);
static int funcs[] = {Output};

void ledlink(void) {
	addarchfile("led", 0666, ledread, ledwrite);
}

static long ledread(Chan* c, void* buf, long n, vlong v) {

	char lbuf[20];
	char *e;

	USED(c);
	e = lbuf + sizeof(lbuf);
	seprint(lbuf, e, "%08ulx%08ulx", ((ulong *)GPLEV)[1], ((ulong *)GPLEV)[0]);
	return readstr(0, buf, n, lbuf);

}

static int checkOnOff(void) {
	char lbuf[20];
	char *e;
	e = lbuf + sizeof(lbuf);
	seprint(lbuf, e, "%08ulx%08ulx", ((ulong *)GPLEV)[1], ((ulong *)GPLEV)[0]);
			
	vlong value, out_val;
	value = strtoull(lbuf, nil, 16);
 	out_val = value & (1 << OUT_GPIO_PIN);
	
	if(out_val == 0) return 0;
	else			return 1;
}

static long ledwrite(Chan *c, void *buff, long n, vlong v) {
	Cmdbuf *cb;
	Cmdtab *ct;
	int prev_state, curr_state, touch_count = 0;
	cb = parsecmd(buff, n);
	if(waserror()) {
		free(cb);
		nexterror();
	}
	
	ct = lookupcmd(cb, ledcmd, nelem(ledcmd));
	
	gpiosel(LED_GPIO_PIN, Output);

	switch(ct->index) {
	case CMDstart:
		print("started \n");
		state = 1;
		gpioout(LED_GPIO_PIN, 0);

		//out_val = lev1 & (1 << OUT_GPIO_PIN);
		//print("Out val for >= 30: %x \n", out_val);

		prev_state = checkOnOff();
		
		int counter = 0;
		while(state == 1) {
			//if(OUT_GPIO_PIN < 30)			out_val = lev0 & (1 << OUT_GPIO_PIN);
			//else						out_val = lev1 & (1 << OUT_GPIO_PIN);

			curr_state = checkOnOff();

			if(prev_state != curr_state) 	touch_count++;
			
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

			counter++;
			prev_state = curr_state;
		}
		
		break;
	
	case CMDstop:
		print("stopped \n");
		state = 0;
		gpioout(LED_GPIO_PIN, 0);
		break;

	case CMDon:
		print("switched on \n");

		state = 0;
		gpioout(LED_GPIO_PIN, 1);
		break;
	
	case CMDoff:
		print("switched off \n");
		state = 0;
		gpioout(LED_GPIO_PIN, 0);
		break;

	case CMDblink:
		print("blinking \n");
		state = 2;

		while(state == 2) {
			gpioout(LED_GPIO_PIN, 1);
			tsleep(&up->sleep, return0, 0, 250);
			gpioout(LED_GPIO_PIN, 0);
			tsleep(&up->sleep, return0, 0, 250);
		}
			
		break;
	}
	free(cb);
	poperror();
	return n;
}

