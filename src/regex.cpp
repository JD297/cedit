#include "regex.hpp"

Regex::Regex(std::string rule, std::string* line, short color):
	rgx(rule),
	it(line->begin(), line->end(), this->rgx),
	color(color)
{

}
