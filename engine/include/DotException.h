#pragma once

#include <stdexcept>
#include <string>

namespace Dot
{
    class RuntimeError : public std::runtime_error
    {
    public:
        RuntimeError(std::string file, int line) noexcept;
        RuntimeError(std::string file, int line, std::string what) noexcept;
        virtual const char* what() const noexcept override;
        virtual std::string getType() const noexcept;
        std::string getOrigin() const noexcept;
        std::string getErrorString() const noexcept;
        const std::string& getFile() const noexcept;
        int getLine() const noexcept;
    protected:
        mutable std::string whatBuffer;
    private:
        int line;
        std::string file;
        std::string errorString;
    };
}

#define DOT_RUNTIME(str) Dot::RuntimeError(__FILE__, __LINE__, str);
#define DOT_RUNTIME_WHAT(err) Dot::RuntimeError(__FILE__, __LINE__, err.what());