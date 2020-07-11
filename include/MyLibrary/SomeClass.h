#pragma once

class SomeClass
{
public:

	SomeClass( int value );

public:

	int Value( void ) const { return value_; }

private:

	int value_;
	
};
