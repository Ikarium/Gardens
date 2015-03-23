#include "Model.h"

#include <stdlib.h>
#include <iostream>
#include <string>


void main(int argc, char* argv[])
{
	std::string session = "82";
	std::string date = "2012-01-01";
	std::string district = "1";
	if (argc > 3)
		district = argv[3];

	if (argc > 2)
		date = argv[2];

	if (argc > 1)
		session = argv[1];

	Model model;
	model.run(date, session, district);
}