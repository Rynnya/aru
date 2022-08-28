#include "convert.hh"

template <> std::string aru::convert::from_string<std::string>(const std::string& str) {
    return str;
}

template <> int aru::convert::from_string<int>(const std::string& str) {
    return std::stoi(str);
}

template <> long aru::convert::from_string<long>(const std::string& str) {
    return std::stol(str);
}

template <> long long aru::convert::from_string<long long>(const std::string& str) {
    return std::stoll(str);
}

template <> unsigned int aru::convert::from_string<unsigned int>(const std::string& str) {
    const auto result = std::stoull(str);
    if (result > std::numeric_limits<unsigned int>::max()) {
        throw std::out_of_range("stou argument out of range");
    }

    return static_cast<unsigned int>(result);
}

template <> unsigned long aru::convert::from_string<unsigned long>(const std::string& str) {
    return std::stoul(str);
}

template <> unsigned long long aru::convert::from_string<unsigned long long>(const std::string& str) {
    return std::stoull(str);
}

template <> float aru::convert::from_string<float>(const std::string& str) {
    return std::stof(str);
}

template <> double aru::convert::from_string<double>(const std::string& str) {
    return std::stod(str);
}

template <> long double aru::convert::from_string<long double>(const std::string& str) {
    return std::stold(str);
}

template <> bool aru::convert::from_string<bool>(const std::string& str) {
    if (str.size() == 1 && str[0] == '1') {
        return true;
    }

    if (str.size() != 4) {
        return false;
    }

    std::string copy = str;
    for (auto& c : copy) {
        c = std::tolower(c);
    }

    return copy == "true";
}
