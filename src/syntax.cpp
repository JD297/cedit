#include "syntax.hpp"

Syntax::Syntax(std::size_t pos, std::string text, short color)
{
	this->pos = pos;
	this->text = text;
	this->color = color;
}

bool Syntax::operator <(const Syntax &syntax) const
{
	return this->pos < syntax.pos;
}

bool Syntax::operator >(const Syntax &syntax) const
{
	return this->pos > syntax.pos;
}

bool Syntax::operator ==(const Syntax &syntax) const
{
	return this->pos == syntax.pos;
}

bool Syntax::operator !=(const Syntax &syntax) const
{
	return this->pos != syntax.pos;
}
