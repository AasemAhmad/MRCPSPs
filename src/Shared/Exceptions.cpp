#include "Shared/Exceptions.hpp"
#include "loguru.hpp"
#include <iostream>
#include <string>

CustomException::CustomException(const std::source_location &src_loc, const std::string &info) : loc(src_loc)
{
    message += info;
}

const char *CustomException::what() const noexcept { return message.c_str(); }

InvalidDataSetException::InvalidDataSetException(const std::source_location &src_loc, const std::string &info)
    : CustomException(src_loc, info)

{
    message = std::format("Invalid dataset {}", source_location_to_string(src_loc));
}

InvalidArgumentException::InvalidArgumentException(const std::source_location &loc, const std::string &info,
                                                   const std::string &arg_name)
    : CustomException(loc), argument_name(arg_name)
{
    message = std::format("Invalid argument {}{}", info, source_location_to_string(loc));
}

std::string InvalidArgumentException::get_argument_name() const { return this->argument_name; }