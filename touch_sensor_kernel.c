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

static long ledwrite(Chan *c, void *buff, long n, vlong v) {
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
		print("started \n");
		state = 1;
		gpioout(LED_GPIO_PIN, 0);

		vlong lev0 = ((vlong *)GPLEV)[0],
		lev1 = ((vlong *)GPLEV)[1],
		out_val;
		
		print("%08x%08x \n", lev1, lev0);

		char lbuf[20];
		char *e;
		e = lbuf + sizeof(lbuf);
		seprint(lbuf, e, "%08ulx%08ulx", ((ulong *)GPLEV)[1], ((ulong *)GPLEV)[0]);

		print("lbuf : %s", lbuf);
			

		out_val = lev0 & (1 << OUT_GPIO_PIN);
		print("Out val for < 30 : %x \n", out_val);

		out_val = lev1 & (1 << OUT_GPIO_PIN);
		print("Out val for >= 30: %x \n", out_val);
		
		int counter = 0;
		while(state == 1) {
			if(OUT_GPIO_PIN < 30)		out_val = lev0 & (1 << OUT_GPIO_PIN);
			else						out_val = lev1 & (1 << OUT_GPIO_PIN);
		
			if(out_val == 0)		gpioout(LED_GPIO_PIN, 0);
			else 				gpioout(LED_GPIO_PIN, 1);

			print("Current out_val : %x \n", out_val);
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

