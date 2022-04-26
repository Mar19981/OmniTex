#pragma once
#include <exception>
#include <string>

class InvalidModelException : public std::exception
{
public:
	InvalidModelException(const std::string_view& m);
	const char* what() const override;
private:
	const std::string_view msg; };

