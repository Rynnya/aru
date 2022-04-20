#pragma once

#include <string>
#include <stdexcept>

namespace aru {

    class convert {
    public:
        template <typename T>
        static T from_string(const std::string& str) {
            throw std::runtime_error("We don't know how to convert this type of objects.");
        }
    };

    template <> std::string convert::from_string<std::string>(const std::string& str);
    template <> int convert::from_string<int>(const std::string& str);
    template <> long convert::from_string<long>(const std::string& str);
    template <> long long convert::from_string<long long>(const std::string& str);
    template <> unsigned int convert::from_string<unsigned int>(const std::string& str);
    template <> unsigned long convert::from_string<unsigned long>(const std::string& str);
    template <> unsigned long long convert::from_string<unsigned long long>(const std::string& str);
    template <> float convert::from_string<float>(const std::string& str);
    template <> double convert::from_string<double>(const std::string& str);
    template <> long double convert::from_string<long double>(const std::string& str);
    template <> bool convert::from_string<bool>(const std::string& str);

}