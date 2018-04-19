#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

//define global constants

enum {
	//pins
	OUT_PIN  = 13,
	LED_PIN = 24
}

enum {
	//commands
	CMDstart,
	CMDstop,
	CMDon,
	CMDoff,
	CMDblink
}

static Cmdtab ledcmd[] = {
	{CMDstart, "start", 1},
	{CMDstop, "stop", 1},
	{CMDon, "on", 1},
	{CMDoff, "off", 1},
	{CMDblink, "blink", 1}
}

static int ledread(Chan*, void*, long, vlong);
static long ledwrite(Chan*, void*, long, vlong);

void ledlink(void) {
	addarchfile("led", 0666, ledread, ledwrite);
}


static int ledread(Chan*, void*, long, vlong) {
	char buff[16];
	vlong value, out_val;

	read(fd, buff, 16);
	buff[16] = 0;
	value = strtoull(buff, nil, 16);
	out_val = value & (1 << out_pin);
	
	if(out_val == 0) return 0;
	else return 1;
}

static long ledwrite(Chan *c, void *buff, long n, vlong) {
	Cmdbuf *cb;
	Cmdtab *ct;
	
	cb = parsecmd(buff, n);
	if(waserror()) {
		free(cb);
		nexterror();
	}
	
	ct = lookupcmd(cb, ledcmd, nelem(ledcmd));

	switch(ct->index) {
	case CMDstart:
		break;
	
	case CMDstop:
		break;

	case CMDon:
		break;

	case CMDblink:
		break;
	}
}
