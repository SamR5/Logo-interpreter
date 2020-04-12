#include "file_parse.h"


std::string fparse::get_file_instructions() {
    std::ifstream file("instructions.txt");
    std::stringstream ss;
    std::string temp;
    bool stopParse(false); // in case of multiline comment
    
    if (file) {
        while (getline(file, temp)) {
            fparse::remove_lead_trail_space(temp);
            if (temp=="#*") {
                stopParse = true;
                continue;
            } else if (temp=="*#") {
                stopParse = false;
                continue;
            }
            if (stopParse || temp=="") { // getline doesn't catch the endline
                continue;
            }
            if (temp[0] != '#') { // if not a comment
                fparse::remove_comment(temp); // the comment at the end
                ss << temp << '\n';
            }
        }
    }
    std::string result = ss.str();
    fparse::remove_lead_trail_space(result);
    std::transform(result.begin(), result.end(), result.begin(), toupper);
    return result;
}

bool fparse::check_change(std::string& original) {
    std::string temp = fparse::get_file_instructions();
    if (temp!=original) {
        original = temp;
        return true;
    }
    return false;
}

void fparse::remove_lead_trail_space(std::string& s) {
    while (s[0]==' ') {
        s.erase(s.begin());
    }
    while (s[s.length()-1]==' ') {
        s.erase(s.end()-1);
    }
    for (int i=s.length(); i>0; i--) {
        if (s[i]==' ' && s[i-1]==' ') {
            s.erase(s.begin()+i);
        }
    }
}

void fparse::remove_comment(std::string& s) {
    int index = s.find('#');
    if (index != -1) {
        s.erase(s.begin()+index, s.end());
    }
}
