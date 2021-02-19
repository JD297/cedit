#ifndef SYNTAX_HPP
#define SYNTAX_HPP

#include <string>

class Syntax
{
public:
	std::size_t pos;
	std::string text;
	short color;

	Syntax(size_t pos, std::string text, short color);

	bool operator <(const Syntax &syntax) const;

	bool operator >(const Syntax &syntax) const;

	bool operator ==(const Syntax &syntax) const;

	bool operator !=(const Syntax &syntax) const;
};

#endif
