#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

/* Hardware text mode color constants. */
enum vga_color
{
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
    return (uint16_t)uc | (uint16_t)color << 8;
}

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t *terminal_buffer;

void terminal_initialize(void)
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t *)0xB8000;
    for (size_t y = 0; y < VGA_HEIGHT; y++)
    {
        for (size_t x = 0; x < VGA_WIDTH; x++)
        {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color)
{
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c)
{
    if (c == '\n')
    {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_row = 0;
    }
    else
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);

    if (++terminal_column == VGA_WIDTH)
    {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_row = 0;
    }
}

void terminal_write(const char *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char *data)
{
    terminal_write(data, strlen(data));
}

#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT 0x3D5
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND 15

void outb(unsigned short port, unsigned char data);
unsigned char inb(unsigned short port);

void fb_move_cursor(unsigned short pos)
{
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,
         ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,
         pos & 0x00FF);
}

void terminal_puthexa(unsigned long n)
{
    if (n / 16)
        terminal_puthexa(n / 16);
    terminal_putchar("0123456789ABCDEF"[n % 16]);
}

void terminal_putnbr(unsigned int n)
{
    if (n / 10)
        terminal_putnbr(n / 10);
    terminal_putchar((n % 10) + '0');
}

bool isCtrlPressed = false;

void ctrlHandling(bool status)
{
    isCtrlPressed = status;
}

void screens(unsigned char s)
{
    if (!isCtrlPressed)
        return;
    terminal_writestring("Switch screen to ");
    terminal_putnbr(s);
}
bool is_caps = false;

void toggle_caps(unsigned char code)
{
    (void)code;
    is_caps = !is_caps;
}
static const unsigned char table[256][2] = {
    [0x02] = {'1', '!'},
    [0x03] = {'2', '@'},
    [0x04] = {'3', '#'},
    [0x05] = {'4', '$'},
    [0x06] = {'5', '%'},
    [0x07] = {'6', '^'},
    [0x08] = {'7', '&'},
    [0x09] = {'8', '*'},
    [0x0A] = {'9', '('},
    [0x0B] = {'0', ')'},
    [0x0C] = {'-', '_'},
    [0x0D] = {'=', '+'},
    [0x1A] = {'[', '{'},
    [0x1B] = {']', '}'},
    [0x27] = {';', ':'},
    [0x28] = {'\'', '\"'},
    [0x29] = {'`', '~'},
    [0x2B] = {'\\', '|'},
    [0x33] = {',', '<'},
    [0x34] = {'.', '>'},
    [0x35] = {'/', '?'},
    [0x39] = {' ', ' '},
    [0x1E] = {'a', 'A'},
    [0x30] = {'b', 'B'},
    [0x2E] = {'c', 'C'},
    [0x20] = {'d', 'D'},
    [0x12] = {'e', 'E'},
    [0x21] = {'f', 'F'},
    [0x22] = {'g', 'G'},
    [0x23] = {'h', 'H'},
    [0x17] = {'i', 'I'},
    [0x24] = {'j', 'J'},
    [0x25] = {'k', 'K'},
    [0x26] = {'l', 'L'},
    [0x32] = {'m', 'M'},
    [0x31] = {'n', 'N'},
    [0x18] = {'o', 'O'},
    [0x19] = {'p', 'P'},
    [0x10] = {'q', 'Q'},
    [0x13] = {'r', 'R'},
    [0x1F] = {'s', 'S'},
    [0x14] = {'t', 'T'},
    [0x16] = {'u', 'U'},
    [0x2F] = {'v', 'V'},
    [0x11] = {'w', 'W'},
    [0x2D] = {'x', 'X'},
    [0x15] = {'y', 'Y'},
    [0x2C] = {'z', 'Z'},
};

void handle_code(unsigned char code)
{
    char c = table[code][is_caps];
    if ((c >= ' ' && c <= '~') ? 1 : 0)
        terminal_putchar(c);
}

void (*func[255])(unsigned char code);

unsigned char get_scan_code()
{
    while (!(inb(0x64) & 0x1))
        ;
    return inb(0x60);
}

void kernel_main(void)
{
    for (size_t i = 0; i < 255; i++)
        func[i] = &handle_code;

    func[0x3a] = &toggle_caps;
    func[0x36] = &toggle_caps;
    func[0xB6] = &toggle_caps;
    func[0x2A] = &toggle_caps;
    func[0xAA] = &toggle_caps;

    /* Initialize terminal interface */
    terminal_initialize();

    /* Newline support is left as an exercise. */
    terminal_writestring("   _  _  ____  \n");
    terminal_writestring(" | || ||___ \\ \n");
    terminal_writestring(" | || |_ __) |\n");
    terminal_writestring(" |__   _/ __/ \n");
    terminal_writestring("    |_||_____|\n");
    terminal_writestring("\n\n");

    while (1)
    {
        // terminal_puthexa(get_scan_code());
        unsigned char code = get_scan_code();
        func[code](code);
    }
}