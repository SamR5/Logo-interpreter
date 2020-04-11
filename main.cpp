/*

LOGO programming language

Parse the instructions in instructions.txt and draw

TODO :
 - clearscreen
 - a way to check if arguments passed to function are correct
*/

#include <GL/gl.h>
#include <GL/glut.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <utility>
#include <map>
#include <algorithm>
#include <regex>
#include <exception>

#define WIDTH 800
#define HEIGHT 600
#define FPS 60
#define PI 3.1415926
#define DEG2RAD PI/180.0

typedef std::pair<float, float> pair2f;
typedef std::vector<std::string> VecStr;

struct Turtle;
struct Function;
class UnknownNameException;// : public std::exception;
class InvalidArgToFunction;// : public std::exception;

void init();
bool is_number(std::string s);
std::string get_file_instructions();
void check_change();
void remove_lead_trail_space(std::string& s);
void remove_comment(std::string& s); // if comment at end of line
void to_mainSet();
bool extract_functions(VecStr set);
bool move_turtle(VecStr set, Turtle& T); // return false on exception
void reset();

void display_callback();
void reshape_callback(int width, int height);
void timer_callback(int);

VecStr longNames = {"FORWARD", "BACKWARD", "RIGHT", "LEFT", "PENUP", "PENDOWN", "SETXY", "REPEAT"};
VecStr shortNames = {"FD", "BD", "RT", "LT", "PU", "PD", "SETXY", "REPEAT"};
VecStr oneInt = {"FD", "BD", "RT", "LT", "REPEAT"}; // instructions that require one integer after
VecStr twoInt = {"SETXY"}; // instructions that require two integer after
VecStr noInt = {"PU", "PD", "TO", "[", "]", "END"}; // instructions that require a non-integer after

std::string instructions;
VecStr mainSet;
std::vector<pair2f> path;
std::vector<std::vector<pair2f>> paths;

struct Turtle {
    float x;
    float y;
    int angle;
    bool isDown;
    bool savePath;
    void fd(int value) {
        x += value * std::cos(angle*DEG2RAD);
        y += value * std::sin(angle*DEG2RAD);
        if (isDown && savePath) {
            path.push_back({x, y});
        }
    }
    void bd(int value) {
        return fd(-value);
    }
    void lt(int value) {
        angle += value;
    }
    void rt(int value) {
        angle -= value;
    }
    void up() {
        isDown = false;
        if (savePath) {
            paths.push_back(path);
            path.clear();
        }
    }
    void down() {
        isDown = true;
        if (savePath) {
            path.push_back({x, y});
        }
    }
    void setxy(int x_, int y_) {
        x = x_;
        y = y_;
        if (isDown && savePath) {
            path.push_back({x, y});
        }
    }
} mainT;

struct Function {
    VecStr parameters;
    VecStr instructions;
    VecStr get_instructions(VecStr arguments) {
        VecStr newInstructions = instructions;
        for (int i=0; i<arguments.size(); i++) {
            std::replace(newInstructions.begin(), newInstructions.end(), 
                         parameters[i], arguments[i]);
        }
        return newInstructions;
    }
    bool is_valid(VecStr arguments) {
        VecStr newInst = get_instructions(arguments);
        Turtle tempT;
        tempT.x = tempT.y = tempT.angle = 0;
        tempT.isDown = true;
        tempT.savePath = false;
        return move_turtle(newInst, tempT);
    }
    void show() {
        std::cout << "parameters: ";
        for (auto i : parameters)
            std::cout << i << " ";
        std::cout << std::endl;
        std::cout << "instructions: ";
        for (auto i : instructions)
            std::cout << i << " ";
        std::cout << std::endl;
    }
};
std::map<std::string, Function> functions;

class UnknownNameException : public std::exception {
    private:
      const char* unknownName;
      const char* functionName;
      const char* infos;
    public:
      UnknownNameException(const char* name, const char* func_="", const char* info_="") :
        unknownName(name), functionName(func_), infos(info_) {}
      const char* what() {
          return "Unknown name";
      }
      const char* get_func() const {
          return functionName;
      }
      const char* get_info() const {
          return infos;
      }
};

class InvalidArgToFunction : public std::exception {
    private:
      std::string functionName;
      VecStr parameters;
      const VecStr arguments;
    public:
      InvalidArgToFunction(std::string name, VecStr param, VecStr args) :
        functionName(name), arguments(args), parameters(param) {}
      const char* what() {
          return "Invalid argument to function";
      }
      std::string get_param_str() const {
          std::string s;
          for (auto p : parameters) {
              s += p + " ";
          }
          return s;
      }
      std::string get_args_str() const {
          std::string s;
          for (auto a : arguments) {
              s += a + " ";
          }
          return s;
      }
};

class InvalidFunctionDefinition : public std::exception {
    private:
      std::string functionName;
    public:
      InvalidFunctionDefinition(std::string name) : functionName(name) {}
      const char* what() {
          return "Invalid function definition";
      }
      std::string get_func() {
          return functionName;
      }
};

void init() {
    glClearColor(0.2, 0.3, 0.4, 1.0);
    reset();
}

bool is_number(std::string s) {
    int i(0);
    if (s[0] == '-') {
        i++;
    }
    for (; i<s.length(); i++) {
        if (!std::isdigit(s[i]))
            return false;
    }
    return true;
}

int to_int(std::string s) {
    for (int i=0; i<s.size(); i++) {
        if (s[i]<'0' || s[i]>'9') {
            if (i==0 && s[i]=='-') continue;
            throw std::invalid_argument("stoi");
        }
    }
    return std::stoi(s);
}

std::string get_file_instructions() {
    std::ifstream file("instructions.txt");
    std::stringstream ss;
    std::string temp;
    bool stopParse(false); // in case of multiline comment
    
    if (file) {
        while (getline(file, temp)) {
            remove_lead_trail_space(temp);
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
                remove_comment(temp); // the comment at the end
                ss << temp << '\n';
            }
        }
    }
    std::string result = ss.str();
    remove_lead_trail_space(result);
    std::transform(result.begin(), result.end(), result.begin(), toupper);
    return result;
}

void check_change() {
    std::string temp = get_file_instructions();
    if (temp!=instructions) {
        instructions = temp;
        reset();
    }
}

void remove_lead_trail_space(std::string& s) {
    while (s[0]==' ') {
        s.erase(s.begin());
    }
    while (s[s.length()-1]==' ') {
        s.erase(s.begin()+s.length()-1);
    }
    for (int i=s.length(); i>0; i--) {
        if (s[i]==' ' && s[i-1]==' ') {
            s.erase(s.begin()+i);
        }
    }
}

void remove_comment(std::string& s) {
    int index = s.find('#');
    if (index != -1) {
        s.erase(s.begin()+index, s.end());
    }
}

void to_mainSet() {
    std::string s;
    bool nextLineIsEndOfArg(false);
    for (int i=0; i<instructions.length(); i++) {
        if (instructions[i] != ' ' && instructions[i] != '\n') {
            if (instructions[i] == '[') {
                mainSet.push_back("[");
            } else if (instructions[i] == ']') {
                mainSet.push_back(s);
                mainSet.push_back("]");
                s.clear();
            } else if (instructions[i] == ':') {
                mainSet.push_back(":");
                nextLineIsEndOfArg = true;
            } else {
                s += instructions[i];
            }
        } else {
            mainSet.push_back(s);
            if (nextLineIsEndOfArg && instructions[i] == '\n') {
                mainSet.push_back("END_OF_ARGS");
                nextLineIsEndOfArg = false;
            }
            s.clear();
        }
    }
    if (s!="") { // don't remember why I wrote this
        std::cout << s << std::endl;
        mainSet.push_back(s);
    }
    // replace the long words with abreviations
    for (int i=0; i<longNames.size(); i++) {
            std::replace(mainSet.begin(), mainSet.end(), longNames[i], shortNames[i]);
    }
    /*
    for (auto i : mainSet) {
        std::cout << i << '-';
    }
    std::cout << std::endl;
    */
}

bool extract_functions(VecStr set) {
    for (int i=0; i<set.size(); i++) {
        if (set[i] == "TO") {
            try {
                Function func;
                int toSkip(2);
                if (set[i+2] == ":") { // if the function takes arguments
                    toSkip++;
                    while (set[i+toSkip] != "END_OF_ARGS") {
                        func.parameters.push_back(set[i+toSkip]);
                        toSkip++;
                    }
                } else { // if the functions doesn't takes arguments
                    toSkip--;
                }
                // first item after arguments
                toSkip++;
                while (set[i+toSkip] != "END") {
                    if (set[i+toSkip] == "TO" || toSkip+i >= set.size()) {
                        throw InvalidFunctionDefinition(set[i+1]);
                    }
                    func.instructions.push_back(set[i+toSkip]);
                    toSkip++;
                }
                functions[set[i+1]] = func;
                i += toSkip;
            } catch (InvalidFunctionDefinition& ifd) {
                std::cerr << "Error (" << ifd.what() << ") >> ";
                std::cerr << ifd.get_func() << std::endl;
                return false;
            }
        }
    }
    return true;
}

bool move_turtle(VecStr set, Turtle& T) {
    /*for (auto i : set) {
        std::cout << i << "-";
    }
    std::cout << std::endl;*/
    int i; // to use it after if exception
    try {
        for (i=0; i<set.size()-1; i++) {
            if (set[i] == "FD") {
                T.fd(to_int(set[i+1])); i++; // to skip the next item
            } else if (set[i] == "BD") {
                T.bd(to_int(set[i+1])); i++;
            } else if (set[i] == "RT") {
                T.rt(to_int(set[i+1])); i++;
            } else if (set[i] == "LT") {
                T.lt(to_int(set[i+1])); i++;
            } else if (set[i] == "REPEAT") {
                int count(-1); // to count '[]' in case they are nested
                int toSkip(0); // items to skip depending on the length
                for (int j=i+3; j<set.size(); j++) {
                    if (set[j] == "[") {
                        count--;
                    } else if (set[j] == "]") {
                        count++;
                    }
                    toSkip++;
                    if (count==0) {
                        break;
                    }
                }
                VecStr subset(set.begin()+i+3, set.begin()+i+3+toSkip);
                for (int x=0; x<to_int(set[i+1]); x++) {
                    if (!move_turtle(subset, T)) {
                        return false;
                    }
                }
                i += toSkip;
            } else if (set[i] == "PU") {
                T.up();
            } else if (set[i] == "PD") {
                T.down();
            } else if (set[i] == "SETXY") {
                T.setxy(to_int(set[i+1]), to_int(set[i+2]));
                i += 2;
            } else if (set[i] == "TO") {
                while (set[i] != "END")
                    i++;
            } else if (functions.find(set[i]) != functions.end()) { // if the function is known
                auto f = functions.find(set[i]);
                VecStr args;
                try {
                    for (int arg=0; arg<f->second.parameters.size(); arg++) {
                        args.push_back(set[i+1+arg]);
                    }
                    if (!f->second.is_valid(args)) {
                        std::vector<char> buffer(set[i].begin(), set[i].end());
                        //throw InvalidArgToFunction(&buffer[0], args);
                        throw InvalidArgToFunction(f->first, f->second.parameters, args);
                    }
                    if (!move_turtle(f->second.get_instructions(args), T)) {
                        return false;
                    }
                    i += f->second.parameters.size();
                } catch (InvalidArgToFunction& iatf) {
                    std::cerr << "Error (" << iatf.what() << ") >> " << f->first << std::endl;
                    std::cerr << "Name: " << f->first << std::endl;
                    std::cerr << "Parameters: " << iatf.get_param_str() << std::endl;
                    std::cerr << "Arguments: " << iatf.get_args_str() << std::endl;
                    return false;
                }
            } else if (set[i] != "]" && set[i] != "[" && set[i] != " " && !is_number(set[i])) {
                
                std::vector<char> buffer(set[i].begin(), set[i].end());
                throw UnknownNameException(&buffer[0]);
            }
        }
    } catch (const std::invalid_argument& ia) {
        std::cerr << "Error (Invalid argument to " << ia.what() << ") >> ";
        std::cerr << set[i] << " " << set[i+1] << " " << set[i+2] << std::endl;
        return false;
        
    } catch (UnknownNameException& une) {
        std::cerr << "Error (" << une.what() << ") >> ";
        std::cerr << set[i] << " " << set[i+1] << " " << set[i+2] << std::endl;
        return false;
    }
    return true;
}

void reset() {
    std::cout << "New drawing\n";
    mainSet.clear();
    path.clear();
    paths.clear();
    functions.clear();
    mainT.x = mainT.y = 0;
    mainT.angle = 0;
    mainT.isDown = true;
    mainT.savePath = true;
    path.push_back({mainT.x, mainT.y});
    instructions = get_file_instructions();
    to_mainSet();
    // in case there is an exception at the EOF
    mainSet.push_back(" "); mainSet.push_back(" ");
    if (!extract_functions(mainSet)) {
        return;
    };
    move_turtle(mainSet, mainT);
    mainT.up(); // saves the last path
}

void display_callback() {
    glClear(GL_COLOR_BUFFER_BIT); // clear screen
    // red cross on the center
    glColor3f(1, 0, 0);
    glBegin(GL_LINES);
      glVertex2f(-6, 0); glVertex2f(5, 0);
      glVertex2f(0, -5); glVertex2f(0, 6);
    glEnd();
    glColor3f(1, 1, 1);
    for (std::vector<pair2f> p : paths) {
        glBegin(GL_LINE_STRIP);
        for (int i=0; i<p.size(); i++) {
            glVertex2f(p[i].first, p[i].second);
        }
        glEnd();
    }
    glutSwapBuffers(); // display / update
}

void reshape_callback(int width, int height) {
    glViewport(0, 0, (GLsizei)width, (GLsizei) height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-width/2, width/2, -height/2, height/2, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

void timer_callback(int) {
    glutPostRedisplay(); // run the display_callback function
    check_change();
    glutTimerFunc(1000/FPS, timer_callback, 0);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv); // initialize
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    //glutInitWindowPosition(); // optional
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("LOGO programming");
    glutDisplayFunc(display_callback);
    glutReshapeFunc(reshape_callback);
    glutTimerFunc(0, timer_callback, 0);
    
    init();
    
    glutMainLoop();
    return 0;
}
