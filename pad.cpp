#include "pad.h"

void pad::WriteTXDATA(uint8_t data) {
	switch (data) {
	case 0:
		break;
	case 1:
		bytes_read = 0;
		joy_stat |= 0b010;
		rx_data_fifo[0] = 0;
		break;
	case 0x42:
	case 0x43: {
		bytes_read = 0;
		joy_stat |= 0b010;
		read_response = true;
		if ((joy_ctrl & 0x2002) == 2) {
			if (pad1_connected) {
				if (pad1_type == "Digital") {
					rx_data_fifo[0] = 0x41;
					rx_data_fifo[1] = 0x5A;
					rx_data_fifo[2] = P1buttons & 0xff;
					rx_data_fifo[3] = (P1buttons >> 8) & 0xff;
				}
				else if (pad1_type == "Mouse") {
					auto& io = ImGui::GetIO();
					int leftClick = ImGui::IsMouseDown(ImGuiMouseButton_Left) ? 0 : 1;
					int rightClick = ImGui::IsMouseDown(ImGuiMouseButton_Right) ? 0 : 1;
					uint16_t buttons = (leftClick << 1) | rightClick;
					rx_data_fifo[0] = 0x12;
					rx_data_fifo[1] = 0x5A;
					rx_data_fifo[2] = 0xff;
					rx_data_fifo[3] = 0xf0 | (buttons << 2);
					rx_data_fifo[4] = io.MouseDelta.x;
					rx_data_fifo[5] = io.MouseDelta.y;
				}
			}
		}
		else if ((joy_ctrl & 0x2002) == 0x2002) {
			if (pad2_connected) {
				if (pad2_type == "Digital") {
					rx_data_fifo[0] = 0x41;
					rx_data_fifo[1] = 0x5A;
					rx_data_fifo[2] = P2buttons & 0xff;
					rx_data_fifo[3] = (P2buttons >> 8) & 0xff;
				}
				else if (pad2_type == "Mouse") {
					auto& io = ImGui::GetIO();
					int leftClick = ImGui::IsMouseDown(ImGuiMouseButton_Left) ? 0 : 1;
					int rightClick = ImGui::IsMouseDown(ImGuiMouseButton_Right) ? 0 : 1;
					uint16_t buttons = (leftClick << 1) | rightClick;
					rx_data_fifo[0] = 0x12;
					rx_data_fifo[1] = 0x5A;
					rx_data_fifo[2] = 0xff;
					rx_data_fifo[3] = 0xf0 | (buttons << 2);
					rx_data_fifo[4] = io.MouseDelta.x;
					rx_data_fifo[5] = io.MouseDelta.y;
				}
			}
		}
		break;
	}
	case 0x03: break;
	case 0x44: read_response = false; break;
	case 0x4d: read_response = false; break;
	case 0xff: break;
	default:
		printf("[PAD] Received unhandled command 0x%x\n", data);
		exit(0);
	}
}

uint8_t pad::ReadRXFIFO() {
	if (read_response) {
		//joy_stat &= ~0b010;
		return rx_data_fifo[bytes_read++];
	}
	else return 0;
}