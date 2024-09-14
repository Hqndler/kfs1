#include "kernel.h"

void kernel_main(void) {
	for (size_t i = 0; i < 255; i++)
		func[i] = &handle_code;

	func[0x3a] = &toggle_caps;
	func[0x36] = &toggle_caps;
	func[0xB6] = &toggle_caps;
	func[0x2A] = &toggle_caps;
	func[0xAA] = &toggle_caps;
	func[0x1D] = &toggle_ctrl;
	func[0x9D] = &toggle_ctrl;
	func[0x45] = &toggle_num;
	func[0xE0] = &handle_extended;
	func[0x0E] = &delete_char;

	for (size_t i = 0; i < 10; i++)
		kmemset(screen_buffer[i], ' ', VGA_HEIGHT * VGA_WIDTH);
	kmemset(screen_cursor, 0, 10 * sizeof(size_t));

	/* Initialize terminal interface */
	vga_init();
	init_buffers();
	terminal_initialize();

	while (1) {
		unsigned char code = get_scan_code();
		func[code](code);
	}
}
