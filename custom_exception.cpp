#include "custom_exception.h"

UnknownName::UnknownName(const char* name, const char* func_, const char* info_) :
        unknownName(name), functionName(func_), infos(info_) {}

const char* UnknownName::what() {
    return "Unknown name";
}

const char* UnknownName::get_func() const {
    return functionName;
}

const char* UnknownName::get_info() const {
    return infos;
}


InvalidArgToFunction::InvalidArgToFunction(std::string name, VecStr param, VecStr args) :
    functionName(name), parameters(param), arguments(args) {}

const char* InvalidArgToFunction::what() {
    return "Invalid argument to function";
}

std::string InvalidArgToFunction::get_param_str() const {
    std::string s;
    for (auto p : parameters) {
        s += p + " ";
    }
    return s;
}

std::string InvalidArgToFunction::get_args_str() const {
    std::string s;
    for (auto a : arguments) {
        s += a + " ";
    }
    return s;
}


InvalidFunctionDefinition::InvalidFunctionDefinition(std::string name) : functionName(name) {}

const char* InvalidFunctionDefinition::what() {
    return "Invalid function definition";
}

std::string InvalidFunctionDefinition::get_func() {
    return functionName;
}

BracketNotMatching::BracketNotMatching(int counter) : count(counter) {}

const char* BracketNotMatching::what() {
    if (count < 0) {
        return "Bracket not closed";
    } else {
        return "Bracket not opened";
    }
}

UselessNumber::UselessNumber() {}

const char* UselessNumber::what() {
    return "Isolated number";
}
