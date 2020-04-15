#ifndef CUSTOM_EXCEPTION_H
#define CUSTOM_EXCEPTION_H

#include <vector>
#include <string>
#include <exception>

typedef std::vector<std::string> VecStr;

class UnknownName : public std::exception {
    private:
      const char* unknownName;
      const char* functionName;
      const char* infos;
    public:
      UnknownName(const char* name, const char* func_="", const char* info_="");
      const char* what();
      const char* get_func() const;
      const char* get_info() const;
};

class InvalidArgToFunction : public std::exception {
    private:
      std::string functionName;
      VecStr parameters;
      const VecStr arguments;
    public:
      InvalidArgToFunction(std::string name, VecStr param, VecStr args);
      const char* what();
      std::string get_param_str() const;
      std::string get_args_str() const;
      std::string get_name() const;
};

class InvalidFunctionDefinition : public std::exception {
    private:
      std::string functionName;
    public:
      InvalidFunctionDefinition(std::string name);
      const char* what();
      std::string get_func();
};

class BracketNotMatching : public std::exception {
    private:
      int count;
    public:
      BracketNotMatching(int counter);
      const char* what();
};

class UselessNumber : public std::exception {
    public:
      UselessNumber();
      const char* what();
};

#endif // CUSTOM_EXCEPTION_H
