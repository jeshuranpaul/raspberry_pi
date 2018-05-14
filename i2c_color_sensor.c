//Device: TCS3472 - RGB Color Sensor with IR filter and White LED

#include "u.h"
#include "libc.h"

void main() {
	uchar buff[10];
	int fdata, fgpio;
	vlong n;
	int clear_regs, red_regs, green_regs, blue_regs, sum;
	int red, green, blue;
	int threshold = 1000;
	char cmd_addr[3] = {0xA0, 0x03, 0xB3};

	bind("#J29", "/dev", MAFTER);
	bind("#G", "/dev", MAFTER);

	fdata = open("/dev/i2c.29.data", ORDWR);
	fgpio = open("/dev/gpio", ORDWR);
	
	fprint(fgpio, "function 13 out \n");
	fprint(fgpio, "set 24 0 \n");

	buff[0] = 0xA0;
	pwrite(fdata, buff, 1, 0);

	buff[0] = 0x03;
	pwrite(fdata, buff, 1, 0);
	
	buff[0] = 0xB2;
	pwrite(fdata, buff, 1, 0);

	while(1) {
		sleep(250);

		//fetching 10 bytes of data, starting from B2
		pread(fdata, buff, 10, 0);
		
		if(buff[0] != 0x44) { 
			print("Incorrect device\n"); 
			break; 
		}
		else if((buff[1] & 1) != 1) { 
			print("Device not ready\n"); 
			continue;
		}
		else {
			clear_regs = (int )buff[3] << 8  | (int)buff[2],
			red_regs =  (int )buff[5] << 8  | (int)buff[4],
			green_regs =  (int )buff[7] << 8  | (int)buff[6],
			blue_regs =  (int )buff[9] << 8  | (int)buff[8];
			
			sum = red_regs + green_regs + blue_regs;
			red 		= (int)(((double)red_regs / (double) sum) * 255.0);
			green 	= (int)(((double)green_regs / (double) sum) * 255.0);
			blue 		= (int)(((double)blue_regs / (double) sum) * 255.0);

			print("%d %d %d %d \t %d %d %d\n", clear_regs, red_regs, green_regs, blue_regs, red, green, blue);

			/*Application: Turn on the lights when it gets dark 
			In this program, I am turning on the touch sensor led when the room is dark and switching off the led when the room is lit.
			Just by trial and error, I've observed that clear values are less than 1000 when the room is dark
			Therefore I am setting the threshold as 1000 */

			if(clear_regs < threshold) {
				fprint(fgpio, "set 24 1 \n");
			}
			else {
				fprint(fgpio, "set 24 0 \n");
			}
		}
	}
}
