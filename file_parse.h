#ifndef FILE_PARSE_H
#define FILE_PARSE_H

#include <string>
#include <algorithm>
#include <sstream>
#include <fstream>

namespace fparse {

std::string get_file_instructions();
bool check_change(std::string& original); // if change, it will be automatically applied
void remove_lead_trail_space(std::string& s);
void remove_comment(std::string& s); // if comment at end of line

}

#endif // FILE_PARSE_H
