#include "kernel.h"

void	*ft_memset(void *pointer, uint8_t value, size_t count)
{
	uint8_t	*str;

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

void	*ft_memshift(void *source, const uint8_t byte, size_t pos, size_t size)
{
	if (pos >= size)
		return source;

	for (size_t i = size; i > pos; --i)
		((uint8_t *)source)[i] = ((uint8_t *)source)[i - 1];
	((uint8_t *)source)[pos] = byte;
	return source;
}

void	*ft_memmove(void *destination, const void *source, size_t size)
{
	size_t			i;
	unsigned char	*d;
	unsigned char	*s;

	d = (unsigned char *) destination;
	s = (unsigned char *) source;
	i = -1;
	if (d > s)
		while (size-- > 0)
			d[size] = s[size];
	else
		while (++i < size)
			d[i] = s[i];
	return (destination);
}