#include <iostream>
#include <string>
#include "Elements/pipe.h"



/**
 *  An application for testing the pipe class.
 *  Should return 0 for passing, any other value
 *  fails.
 */
int main (void )
{

	std::string expected = "myPipe";
	Pipe p = Pipe( expected );

	// see that the pipe has the correct name
	std::cout << "Created a pipe named " << p.name << std::endl;

	std::string actual = p.name;

	int value = actual.compare(expected);

    return value;
}
