#include "controller.h"
controller::controller() {

}
void controller::exec() {
	switch (joy_tx_data) {
	case 0x01: {
		joy_rx_data = 0x69; // "dummy response"
		joy_ctrl &= ~1; // Don't ask why I do this
		break;
	}
	case 0x42: {
		joy_rx_data = id & 0xff;
		break;
	}
	case 0x0: {
		if (joy_rx_data == (id & 0xff)) {	// Previous command was 42h
			joy_rx_data = (id & 0xff00) >> 8;
		}
		else if (joy_rx_data == ((id & 0xff00) >> 8)) {
			joy_rx_data = buttons & 0xff;
		}
		else {
			joy_rx_data = (buttons & 0xff00) >> 8;
		}
		break;
	}
	default: {
		printf("[PAD] Unhandled JOY_TX_DATA command\n");
		exit(0);
	}
	}
}