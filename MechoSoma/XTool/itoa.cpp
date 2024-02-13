#include "xtool.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

/**

 * C++ version char* style "itoa":

 */

const char* port_itoa( int value, char* result, int base ) {
	// check that the base if valid
	if (base < 2 || base > 16) {
		*result = 0;
		return result;
	}

	char* out = result;
	int quotient = value;

	do {
		*out = "0123456789abcdef"[ std::abs( quotient % base ) ];
		++out;
		quotient /= base;

	} while ( quotient );

	// Only apply negative sign for base 10
	if ( value < 0 && base == 10)
		*out++ = '-';

	std::reverse( result, out );
	*out = 0;
	return result;
}

const char* port_ltoa( long value, char* result, int base ) {
	// check that the base if valid
	if (base < 2 || base > 16) {
		*result = 0;
		return result;
		}

	char* out = result;
	long quotient = value;

	do {
		*out = "0123456789abcdef"[ std::abs( quotient % base ) ];
		++out;
		quotient /= base;
	} while ( quotient );

	// Only apply negative sign for base 10
	if ( value < 0 && base == 10)
		*out++ = '-';

	std::reverse( result, out );
	*out = 0;
	return result;
}

const char* port_ultoa( unsigned long value, char* result, int base ) {
	// check that the base if valid
	if (base < 2 || base > 16) {
		*result = 0;
		return result;
	}

	char* out = result;
	unsigned long quotient = value;

	do {
		*out = "0123456789abcdef"[ quotient % base ];
		++out;
		quotient /= base;
	} while ( quotient );

	std::reverse( result, out );
	*out = 0;
	return result;
}
