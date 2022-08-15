#include "dot_Exception.h"

#include <sstream>

namespace dot
{
    RuntimeError::RuntimeError(std::string file, int line) noexcept
        : file(file), line(line), std::runtime_error(""){}

    RuntimeError::RuntimeError(std::string file, int line, std::string errorString) noexcept
        : file(file), line(line), errorString(errorString), std::runtime_error(errorString){}

    const char* RuntimeError::what() const noexcept
    {
        std::ostringstream oss;
        oss << '\n' 
            << getType() << '\n'
            << getOrigin() << '\n'
            << getErrorString() << '\n';

        whatBuffer = oss.str();

        return whatBuffer.c_str();
    }

    std::string RuntimeError::getType() const noexcept
    {
        return "\t[Exception Type] Dot Exception";
    }

    std::string RuntimeError::getOrigin() const noexcept
    {
        std::ostringstream oss;
        oss << '\t' << "[File] " << getFile() << '\n'
            << '\t' << "[Line] " << getLine();
        
        return oss.str();
    }

    std::string RuntimeError::getErrorString() const noexcept
    {
        std::ostringstream oss;
        oss << '\t' << "[Error String] " << errorString;

        return oss.str();
    }

    const std::string& RuntimeError::getFile() const noexcept
    {
        return file;
    }

    int RuntimeError::getLine() const noexcept
    {
        return line;
    }
}