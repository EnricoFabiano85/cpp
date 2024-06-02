#include "DDouble.h"
#include "Operators.h"

#include<iostream>

int main()
{
	DDouble<TANGENT> x{2, .322};
	DDouble<TANGENT> x1{1, .14};
	DDouble<TANGENT> y{5, 0};
	DDouble<TANGENT> z{10, 0};
	DDouble<TANGENT> k{42, 0};

  // auto t = (x/x1);

	DDouble<TANGENT> t = k+sin((z+y)/(x+x1));



	std::cout << t.primal() 
  << " " << t.sensitivity()  
  <<  std::endl;
}
