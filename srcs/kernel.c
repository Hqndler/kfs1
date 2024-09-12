#include "kernel.h"

#define pos(x, y) ({return x * VGA_WIDTH + y};)
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT 0x3D5
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND 15

void outb(unsigned short port, unsigned char data);
unsigned char inb(unsigned short port);

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

uint8_t terminal_color;
uint16_t *terminal_buffer;

uint8_t kernel_screen = 0;
uint8_t screen_buffer[10][2000];
size_t screen_cursor[10];

void vga_init()
{
    terminal_buffer = (uint16_t *)0xB8000;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++)
        terminal_buffer[i] = vga_entry(' ', terminal_color);
}

void fb_move_cursor(unsigned short pos)
{
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,
         ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,
         pos & 0x00FF);
}

void terminal_initialize(void)
{
    for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++)
        terminal_buffer[i] = vga_entry(screen_buffer[kernel_screen][i], terminal_color);
    fb_move_cursor(screen_cursor[kernel_screen]);
}

void terminal_setcolor(uint8_t color)
{
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t index)
{
    terminal_buffer[index] = vga_entry(c, color);
    kmemshift(screen_buffer[kernel_screen], c, index, 2000);
}

void terminal_putchar(char c)
{
    if (c == '\n')
    {
        screen_cursor[kernel_screen] += VGA_WIDTH;
        screen_cursor[kernel_screen] -= screen_cursor[kernel_screen] % VGA_WIDTH;
    }
    else
        terminal_putentryat(c, terminal_color, screen_cursor[kernel_screen]++);
    if (screen_cursor[kernel_screen] >= (VGA_WIDTH * VGA_HEIGHT) - 1)
    {
        // kmemmove(&screen_buffer[kernel_screen], &screen_buffer[kernel_screen][VGA_WIDTH], VGA_WIDTH * VGA_HEIGHT * sizeof(uint8_t));
        kmemmove(&screen_buffer[kernel_screen][VGA_WIDTH * 6], &screen_buffer[kernel_screen][VGA_WIDTH * 7], VGA_WIDTH * VGA_HEIGHT * sizeof(uint8_t));
        screen_cursor[kernel_screen] = (VGA_HEIGHT - 1) * VGA_WIDTH;
    }
    fb_move_cursor(screen_cursor[kernel_screen]);
    terminal_initialize();
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

bool is_caps = false;
bool is_ctrl = false;
bool is_num = true;

void toggle_caps(unsigned char code)
{
    (void)code;
    is_caps = !is_caps;
}

void toggle_ctrl(unsigned char code)
{
    (void)code;
    is_ctrl = !is_ctrl;
}

void toggle_num(unsigned char code)
{
    (void)code;
    is_num = !is_num;
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
    [0x1C] = {'\n', '\n'},
    // Keypad
    [0x47] = {'7', '7'},
    [0x48] = {'8', '8'},
    [0x49] = {'9', '9'},
    [0x4A] = {'-', '-'},
    [0x4B] = {'4', '4'},
    [0x4C] = {'5', '5'},
    [0x4D] = {'6', '6'},
    [0x4E] = {'+', '+'},
    [0x4F] = {'1', '1'},
    [0x50] = {'2', '2'},
    [0x51] = {'3', '3'},
    [0x52] = {'0', '0'},
    [0x53] = {'.', '.'},
    [0x37] = {'*', '*'},
};

void switch_screen(uint8_t n)
{
    if (n == kernel_screen)
        return;
    kernel_screen = n;
    terminal_initialize();
    terminal_writestring("\nSwitch Screen to ");
    terminal_putnbr(n);
    terminal_writestring("\n");
}

void handle_code(unsigned char code)
{
    if (code >= 0x47 && code <= 0x53 && code != 0x4A && code != 0x4E && !is_num)
        return;
    char c = table[code][is_caps];
    if (!((c >= ' ' && c <= '~') || c == '\n'))
        return;
    if (is_ctrl && c >= '0' && c <= '9')
        switch_screen(c - '0');
    else
        terminal_putchar(c);
}

void (*func[255])(unsigned char code);

unsigned char get_scan_code()
{
    while (!(inb(0x64) & 0x1))
        ;
    return inb(0x60);
}

void delete_char(unsigned char code)
{
    if (code == 0x0E)
    {
        --screen_cursor[kernel_screen];
    }
    // size_t len = VGA_WIDTH - (screen_cursor[kernel_screen] % VGA_WIDTH);
    size_t len = (VGA_WIDTH * VGA_HEIGHT) - screen_cursor[kernel_screen];
    kmemmove(&screen_buffer[kernel_screen][screen_cursor[kernel_screen]],
             &screen_buffer[kernel_screen][screen_cursor[kernel_screen]] + 1,
             len);
    terminal_initialize();
    fb_move_cursor(screen_cursor[kernel_screen]);
}

void handle_extended(unsigned char code)
{
    code = get_scan_code();
    switch (code)
    {
    case 0x4B:
        // if (screen_cursor[kernel_screen] % VGA_WIDTH != 0)
        if (screen_cursor[kernel_screen] != VGA_WIDTH * 6)
            --screen_cursor[kernel_screen];
        break;
    case 0x4D:
        // if (screen_cursor[kernel_screen] % VGA_WIDTH != VGA_WIDTH - 1)
        if (screen_cursor[kernel_screen] != (VGA_WIDTH * VGA_HEIGHT) - 1)
            ++screen_cursor[kernel_screen];
        break;
    case 0x53:
        delete_char(0x53);
        break;
    case 0x47:
        if (is_ctrl)
            screen_cursor[kernel_screen] = VGA_WIDTH * 6;
        else
            screen_cursor[kernel_screen] -= screen_cursor[kernel_screen] % VGA_WIDTH;
        break;
    case 0x4f:
        if (is_ctrl)
            screen_cursor[kernel_screen] = (VGA_HEIGHT * VGA_WIDTH) - 1;
        else
            screen_cursor[kernel_screen] += (VGA_WIDTH - (screen_cursor[kernel_screen] % VGA_WIDTH)) - 1;
        break;

    case 0x1C:
        terminal_putchar('\n');
        break;
    case 0x35:
        terminal_putchar('/');
        break;

    default:
        break;
    }
    fb_move_cursor(screen_cursor[kernel_screen]);
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
    func[0x1D] = &toggle_ctrl;
    func[0x9D] = &toggle_ctrl;
    func[0x45] = &toggle_num;
    func[0xE0] = &handle_extended;
    func[0x0E] = &delete_char;
    // kmemset(screen_buffer, '@',);
    for (size_t i = 0; i < 10; i++)
    {
        kmemset(screen_buffer[i], ' ', VGA_HEIGHT * VGA_WIDTH);
    }

    kmemset(screen_cursor, 0, 10 * sizeof(size_t));

    /* Initialize terminal interface */
    // terminal_initialize();
    vga_init();

    /* Newline support is left as an exercise. */
    terminal_writestring("  _  _  ____  \n");
    terminal_writestring(" | || ||___ \\ \n");
    terminal_writestring(" | || |_ __) |\n");
    terminal_writestring(" |__   _/ __/ \n");
    terminal_writestring("    |_||_____|\n");
    terminal_writestring("\n\n");

    while (1)
    {
        unsigned char code = get_scan_code();
        // terminal_puthexa(code);
        func[code](code);
    }
}