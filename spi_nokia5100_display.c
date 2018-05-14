//Device: PCD8544 - Graphic LCD 84x48 - Nokia 5110

#include <u.h>
#include <libc.h>


uchar buff[2];
int fd_gpio, fd_spi0;
enum {
	DC_GPIO = 23,
	RST_GPIO = 24
};

void _init(void);
void resetDevice(void);
void configureDevice(void);
void clearDisplay(void);
void setXY(int, int);
void writeData(uchar);
void writeCmd(uchar);
void swap(int*, int*);
void drawRectangle(int, int, int, int);


void main() {
	int x1 = 0, x2 = 47, y1 = 0, y2 = 83;
	
	_init();

	resetDevice();
	configureDevice();
	setXY(0, 0);

	while(1) {
		clearDisplay();
		drawRectangle(x1, y1, x2, y2);
		
		x1++; x2--;
		if(x1 > 47 || x2 < 0) {
			x1 = 0; x2 = 47;
		}
		
		y1++; y2--;
		if(y1 > 83 || y2 < 0) {
			y1 = 0; y2 = 83;
		}

		print("%d   %d   %d   %d \n", x1, y1, x2, y2);
		sleep(100);
	}
}

void clearDisplay(void) {
	fprint(fd_gpio, "set %d 1 \n", DC_GPIO);

	for(int i = 0; i < 84 * 6; i++) {
		buff[0] = 0x00;
		pwrite(fd_spi0, buff, 1, 0);
	}
}

void setXY(int row, int col) {
	fprint(fd_gpio, "set %d 0 \n", DC_GPIO);

	//setting bank/row
	buff[0] = 0x40 | row; 
	pwrite(fd_spi0, buff, 1, 0);
	
	//setting col
	buff[0] = 0x80 | col;	
	pwrite(fd_spi0, buff, 1, 0);
}

void writeData(uchar c) {
	fprint(fd_gpio, "set %d 1 \n", DC_GPIO);
	buff[0] = c;
	pwrite(fd_spi0, buff, 1, 0);
}

void writeCmd(uchar c) {
	fprint(fd_gpio, "set %d 0 \n", DC_GPIO);
	buff[0] = c;
	pwrite(fd_spi0, buff, 1, 0);
}

void configureDevice(void) {
	writeCmd(0x21);	//turning on advanced commands
	writeCmd(0xa4);	//setting LCD Vo
	writeCmd(0xbf);	//setting contrast
	writeCmd(0x14);	//setting LCD bias mode 1:40
	writeCmd(0x20);	//Function set => PD=0, V=0, H=0
	writeCmd(0x0c);	//display control => (D, E) = (1,0) i.e normal mode
}

void resetDevice(void) {
	fprint(fd_gpio, "set %d 1 \n", RST_GPIO);
	sleep(100);
	fprint(fd_gpio, "set %d 0 \n", RST_GPIO);
	sleep(100);
	fprint(fd_gpio, "set %d 1 \n", RST_GPIO);
	sleep(100);
}

void _init(void) {
	bind("#G", "/dev", MAFTER);
	bind("#π", "/dev", MAFTER);
	
	fd_gpio = open("/dev/gpio", ORDWR);
	fd_spi0 = open("/dev/spi0", ORDWR);
	
	fprint(fd_gpio, "function %d out \n", DC_GPIO);
	fprint(fd_gpio, "function %d out \n", RST_GPIO);
	
}

void swap(int *a, int *b) {
	int temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

void drawRectangle(int row1, int col1, int row2, int col2) {

	uchar hor_helper[] = 	{0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80},
	ver_helper[] = 				{0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};

	if(row1 > 47 || row2 > 47 || col1 > 83 || col2 > 83)  {
		print("Row/column indexes out of bounds \n");
		return;
	}

	else {
		if(row1 > row2) 	swap(&row1, &row2);
		if(col1 > col2) 		swap(&col1, &col2);
		
		int bank1 		= row1/8, 
		bank2 				= row2/8,
		active_bit1 	= row1 % 8,
		active_bit2 	= row2 % 8;

		

		uchar val1 = 0 | 1 << active_bit1, 
		val2 = 0 | 1 <<  active_bit2;
		
		//drawing horizontal pixels
		setXY(bank1, col1);
		for(int i = col1; i <= col2; i++) writeData(val1);
		
		setXY(bank2, col1);
		for(int i = col1; i <= col2; i++) writeData(val2);
		
		//setting appropriate top corner pixels
		setXY(bank1, col1);
		writeData(0xFF & hor_helper[active_bit1]);

		setXY(bank1, col2);
		writeData(0xFF & hor_helper[active_bit1]);
		
		//setting appropriate bottom corner pixels
		setXY(bank2, col1);
		writeData(0 | ver_helper[active_bit2]);

		setXY(bank2, col2);
		writeData(0 | ver_helper[active_bit2]);
		
		//drawing remaining vertical pixels
		for(int i = bank1 + 1; i < bank2; i++) {
			setXY(i, col1);
			writeData(0xFF);
			setXY(i, col2);
			writeData(0xFF);
		}
	}
	return;
}
