#include "pad.h"

pad::pad() {
	if((memcard1 = fopen(memcard1_dir, "rb+")) == NULL)
		memcard1 = fopen(memcard1_dir, "w+");
}

void pad::WriteTXDATA(uint8_t data) {
	if (mem_receive_addrmsb) {
		mem_sector = 0;
		mem_sector |= (data << 8);
		checksum = data;
		mem_receive_addrmsb = false;
		mem_receive_addrlsb = true;
		irq = true;
		return;
	}
	if (mem_receive_addrlsb) {
		mem_sector |= data;
		mem_receive_addrlsb = false;
		irq = true;
		bytes_read = 0;
		rx_data_fifo[0] = 0;
		rx_data_fifo[1] = 0x5c;
		rx_data_fifo[2] = 0x5d;
		rx_data_fifo[3] = mem_sector >> 8;
		rx_data_fifo[4] = mem_sector & 0xff;
		response_length = 5;
		reading_sector = true;
		checksum ^= data;
		return;
	}
	switch (data) {
	case 0:
		irq = true;
		joy_stat |= 0b010;
		break;
	case 1:
		reading_sector = false;
		mem_transfer = false;
		calculate_checksum = false;
		mem_receive_addrlsb = false;
		mem_receive_addrmsb = false;
		bytes_read = 0;
		joy_stat |= 0b010;
		//joy_stat |= (1 << 7);
		if ((joy_ctrl & 0x2002) == 2) {
			if (pad1_connected) irq = true;
		}
		else if ((joy_ctrl & 0x2002) == 0x2002) {
			if (pad2_connected) irq = true;
		}
		rx_data_fifo[0] = 0xff;
		rx_data_fifo[1] = 0xff;
		read_response = true;
		response_length = 0;
		break;
	case 0x81:
		bytes_read = 0;
		joy_stat |= 0b010;
		joy_stat |= (1 << 7);
		irq = true;
		rx_data_fifo[0] = 0x0;
		read_response = true;
		mem_transfer = true;
		break;
	case 0x52:
		bytes_read = 0;
		if ((joy_ctrl & 0x2002) == 2) {
			rx_data_fifo[0] = 0x0;
			rx_data_fifo[1] = 0x5a;
			rx_data_fifo[2] = 0x5d;
		}
		irq = true;
		read_response = true;
		break;
	case 0x43: { // Return 0xff and do not ack because we are emulating a digital pad for now
		joy_stat |= 2;
		read_response = true;
		bytes_read = 0;
		rx_data_fifo[0] = 0xff;
		break;
	}
	case 0x42: {
		bytes_read = 0;
		joy_stat |= 0b010;
		irq = true;
		read_response = true;
		if ((joy_ctrl & 0x2002) == 2) {
			if (pad1_connected) {
				if (pad1_type == "Digital") {
					rx_data_fifo[0] = 0x41;
					rx_data_fifo[1] = 0x5A;
					rx_data_fifo[2] = P1buttons & 0xff;
					rx_data_fifo[3] = (P1buttons >> 8) & 0xff;
					response_length = 3;
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

					response_length = 5;
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
					response_length = 3;
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
					response_length = 5;
				}
			}
		}
		break;
	}
	case 0x03: break;
	case 0x44: read_response = false; break;
	case 0x45: { // Return 0xff and do not ack because we are emulating a digital pad for now
		joy_stat |= 2;
		read_response = true;
		bytes_read = 0;
		rx_data_fifo[0] = 0xff;
		break;
	}
	case 0x4d: read_response = false; break;
	case 0xff: break;
	default:
		printf("[PAD %d] Received unhandled command 0x%x\n", ((joy_ctrl & 0x2002) == 2) ? 1 : 2, data);
		//exit(0);
	}
}

uint8_t pad::ReadRXFIFO() {

	if (read_response) {
		if (bytes_read == 129) { 
			abort_irq = true; 
		}
		if ((bytes_read == response_length) && reading_sector) {
			bytes_read = 0;
			fseek(memcard1, mem_sector * 128, SEEK_SET);
			fread(rx_data_fifo, sizeof(uint8_t), 128, memcard1);
			rx_data_fifo[129] = 0x47;
			reading_sector = false;
			response_length = 130;
			mem_transfer = false;
			calculate_checksum = true;
		}
		if (bytes_read == response_length) {
			joy_stat &= ~0b010;
		}
		uint8_t byte = rx_data_fifo[bytes_read++];
		if (mem_transfer && (byte == 0x5d)) {
			mem_transfer = false;
			mem_receive_addrmsb = true;
		}
		if (calculate_checksum) {
			checksum ^= byte;
			if (bytes_read == 128) {
				calculate_checksum = false;
				rx_data_fifo[128] = checksum;
			}
		}
		if (bytes_read == 130) {
			bytes_read = 0;
			rx_data_fifo[0] = 0;
		}
		return byte;
	}
	else return 0;
}