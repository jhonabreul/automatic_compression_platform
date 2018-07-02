#include <division.h>

float Division::apply(const float x, const float y)
{
	if (y == 0) {
		throw std::overflow_error("Division by zero");
	}

    return x / y;
}