#include "kernel.h"

void	*ft_memset(void *pointer, int value, size_t count)
{
	unsigned char	*str;

	str = pointer;
	while (count--)
	{
		*str = (uint8_t) value;
		str++;
	}
	return (pointer);
}

void	*ft_memcpy(void *destination, const void *source, size_t size)
{
	uint8_t	*d;
	uint8_t	*s;
	size_t   i;

	d = (uint8_t *)destination;
	s = (uint8_t *)source;
	i = -1;
	while (++i < size)
		d[i] = s[i];
	return (destination);
}