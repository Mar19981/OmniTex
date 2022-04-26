#include "InvalidImageException.h"

InvalidImageException::InvalidImageException(const std::string_view& m): msg(m)
{
}

const char* InvalidImageException::what() const
{
    return msg.data();
}
