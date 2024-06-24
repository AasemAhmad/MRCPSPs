#pragma once

#include "Shared/Utils.hpp"
#include <exception>
#include <format>
#include <iostream>
#include <string>

class CustomException : public std::exception
{
  public:
    CustomException(const std::source_location &loc, const std::string &info = "");
    const char *what() const noexcept override;
    ~CustomException() noexcept override = default;

  protected:
    std::source_location loc;
    std::string message = std::format("Exception occurred at {} ", source_location_to_string(loc));
};

class InvalidDataSetException : public CustomException
{
  public:
    InvalidDataSetException(const std::source_location &loc, const std::string &info = "");
};

class InvalidArgumentException : public CustomException
{
  public:
    InvalidArgumentException(const std::source_location &loc, const std::string &info,
                             const std::string &argument_name);
    std::string get_argument_name() const;

  private:
    std::string argument_name;
};
