#include "hw_config.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "codec.h"
#include "asio.h"
#include "led.h"

uint8_t flag_playing;
uint8_t transfer_on = 0;

int main(void)
{
	uint32_t fifo_num;
	uint32_t last_smprate = 0;
	uint8_t  configured = 0;
	
	Set_System();
	Set_USBClock();
	USB_Config();
	USB_Init();
	
	asio_hwinit();
	asio_samplerate(0); /* default 44.1kHz */
	asio_reset();
	asio_fifo_init();
	led_init();
	
	while(1){
		if(bDeviceState == CONFIGURED){
			if(!configured){
				led_on();
				configured = 1;
			}
			
			if(!transfer_on){
				fifo_num = asio_fifo_getnum();
				if(fifo_num >= ASIO_FIFO_SIZE / 2){
					asio_transfer_control(1);
					transfer_on = 1;
					led_blink();
				}
			}
		}else{
			if(configured){
				led_off();
				configured = 0;
			}
		}
		
		if(SAMPLE_RATE != last_smprate){
			switch(SAMPLE_RATE & 0x00FFFFFF){
				case 0x0000AC44: asio_samplerate(0);break;
				case 0x0000BB80: asio_samplerate(1);break;
			}
			last_smprate = SAMPLE_RATE;
		}
	}
	
	return 0;
}
