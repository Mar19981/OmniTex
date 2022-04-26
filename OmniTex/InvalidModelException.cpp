#include "InvalidModelException.h"

InvalidModelException::InvalidModelException(const std::string_view& m): msg(m)
{
}

const char* InvalidModelException::what() const
{
    return msg.data();
}
