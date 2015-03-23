
#include "System.h"

class Model
{
	System sys;

public:

	Model();

	void run(std::string, std::string, std::string);
	void load(std::string, std::string, std::string);
	void save(std::string, std::string, std::string);
};
