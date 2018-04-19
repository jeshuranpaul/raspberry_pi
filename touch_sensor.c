#include <u.h>
#include <libc.h>

int checkOnOff(int, int);

void main() {
	int fd;
	
	int out_pin = 13,
	led_pin = 24,
	touch_count = 0;
	
	int prev_state, curr_state;

	bind("#G", "/dev", MAFTER); 	//MAFTER does the -a part in the echo command
	fd = open("/dev/gpio", ORDWR);
	
	fprint(fd, "function %d out \n", led_pin);
	fprint(fd, "set %d 0 \n", led_pin);

	prev_state = checkOnOff(fd, out_pin);

	while(1)  {
		curr_state = checkOnOff(fd, out_pin);
		
		if(prev_state != curr_state) 
			touch_count++;

		switch(touch_count % 3) {
			case 0:
				fprint(fd, "set %d 0 \n", led_pin);
				break;
			
			case 1:
				fprint(fd, "set %d 1 \n", led_pin);
				break;
			
			case 2:
				fprint(fd, "set %d 0 \n", led_pin);
				sleep(100);
				fprint(fd, "set %d 1 \n", led_pin);
				sleep(100);
				break;
		}

		prev_state = curr_state;
	}
}

int checkOnOff(int fd, int out_pin) {
	char buff[16];
	vlong value, out_val;

	read(fd, buff, 16);
	buff[16] = 0;
	value = strtoull(buff, nil, 16);
	out_val = value & (1 << out_pin);
	
	if(out_val == 0) return 0;
	else return 1;
}
