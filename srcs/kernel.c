#include "kernel.h"

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

uint8_t kernel_screen = 0;
uint8_t screen_buffer[10][2000];

uint8_t cursor = 0;
uint8_t screen_cursor[10];

void terminal_initialize(void)
{
    ft_memset(screen_buffer[kernel_screen], ' ', 2000);
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
    ft_memcpy(screen_buffer[kernel_screen], terminal_buffer, 2000);
}

void terminal_setcolor(uint8_t color)
{
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
    ft_memshift(screen_buffer[kernel_screen], c, index, 2000);
}

void terminal_putchar(char c)
{
    if (c == '\n')
    {
        while (terminal_column <= VGA_WIDTH)
            terminal_putentryat(' ', terminal_color, terminal_column++, terminal_row);
        terminal_column = 0;
        ++terminal_row;
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

void kernel_main(void)
{
    /* Initialize terminal interface */
    terminal_initialize();

    /* Newline support is left as an exercise. */
    terminal_writestring("   _  _  ____  \n");
    terminal_writestring(" | || ||___ \\ \n");
    terminal_writestring(" | || |_ __) |\n");
    terminal_writestring(" |__   _/ __/ \n");
    terminal_writestring("    |_||_____|\n");
    for (size_t i = 1000; i < 2000; ++i)
        terminal_buffer[i] = vga_entry(screen_buffer[kernel_screen][i - 1000], terminal_color);
}