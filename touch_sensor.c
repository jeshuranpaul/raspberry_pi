#include <u.h>
#include <libc.h>

int getValue(int, int);

void main() {
	int fd;
	
	int out_pin = 13,
	led_pin = 24,
	touch_count = 0;
	
	int prev_val, curr_val;
	vlong value;

	bind("#G", "/dev", MAFTER); 	//MAFTER does the -a part in the echo command
	fd = open("/dev/gpio", ORDWR);
	
	fprint(fd, "function %d out \n", led_pin);
	fprint(fd, "set %d 0 \n", led_pin);

	prev_val = getValue(fd, out_pin);

	while(1)  {
		curr_val = getValue(fd, out_pin);
		
		if(prev_val != curr_val) 
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

		prev_val = curr_val;
	}
}

int getValue(int fd, int out_pin) {
	char buff[16];
	vlong value;
	int out_val, out_bit_val;

	int shift_factor = out_pin / 4, 
	shift = shift_factor * 4;

	read(fd, buff, 16);
	buff[16] = 0;
	value = strtoull(buff, nil, 16);
	out_val = value & (1 << out_pin);
	out_bit_val = out_val >> shift;
	return out_bit_val;
}
