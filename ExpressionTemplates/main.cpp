#include "DDouble.h"
#include "Operators.h"

#include<iostream>

int main()
{
	DDouble x{2};
	DDouble x1{1};
	DDouble y{5};
	DDouble z{10};
	DDouble k{42};

	DDouble t = sin((z+y)/(x+x1));

	std::cout << "Value of expression is: " << t.primitive() <<  std::endl;
}
