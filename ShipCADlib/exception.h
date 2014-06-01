#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdexcept>
#include <string>

namespace ShipCADException {

    class ListIndexOutOfBounds : public std::out_of_range
    {
    public:
        ListIndexOutOfBounds(const std::string& what_arg) : std::out_of_range(what_arg) {};
    };

    class PointIndexOutOfBounds : public std::out_of_range
    {
    public:
        PointIndexOutOfBounds(const std::string& what_arg) : std::out_of_range(what_arg) {};
    };

    class IndexOutOfRange : public std::out_of_range
    {
    public:
        IndexOutOfRange(const std::string& what_arg) : std::out_of_range(what_arg) {};
    };
};

#endif // EXCEPTION_H
