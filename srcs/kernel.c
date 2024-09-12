#include "kernel.h"

void kernel_main(void)
{
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
    {
        kmemset(screen_buffer[i], ' ', VGA_HEIGHT * VGA_WIDTH);
    }
    kmemset(screen_cursor, 0, 10 * sizeof(size_t));

    /* Initialize terminal interface */
    vga_init();

    for (size_t i = 0; i < 10; i++)
    {
        /* code */
        kernel_screen = (uint8_t)i;
        write_string_buffer("  _  _  ____  \n");
        write_string_buffer(" | || ||___ \\ \n");
        write_string_buffer(" | || |_ __) |\n");
        write_string_buffer(" |__   _/ __/ \n");
        write_string_buffer("    |_||_____|\n");
        write_string_buffer("tty: ");
        char tmp[2] = {i + '0', '\0'};
        write_string_buffer(tmp);
        write_string_buffer("\n");
    }
    kernel_screen = 0;
    terminal_initialize();

    /* Newline support is left as an exercise. */

    while (1)
    {
        unsigned char code = get_scan_code();
        // terminal_puthexa(code);
        func[code](code);
    }
}