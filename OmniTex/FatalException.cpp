#include "FatalException.h"

FatalException::FatalException(const std::string_view& m): msg(m)
{
}

const char* FatalException::what() const
{
	return msg.data();
}
