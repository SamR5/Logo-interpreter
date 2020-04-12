/*

LOGO programming language

Parse the instructions in instructions.txt and draw

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
#include <ctime>
#include <random>
#include <thread>
#include "custom_exception.h"
#include "file_parse.h"

#define WIDTH 800
#define HEIGHT 600
#define FPS 60
#define PI 3.1415926
#define DEG2RAD PI/180.0

typedef std::pair<float, float> pair2f;
typedef std::vector<std::string> VecStr;

struct Turtle;
struct Function;


void init();
bool is_number(std::string s);
float to_number(std::string s);
int random_range(int a, int b);

void to_mainSet();
bool extract_functions(VecStr set);
bool move_turtle(VecStr set, Turtle& T); // return false on exception
void reset();

void display_callback();
void reshape_callback(int width, int height);
void timer_callback(int);
void keyboard_callback(unsigned char key, int x, int y);

VecStr longNames = {"FORWARD", "BACKWARD", "RIGHT", "LEFT", "PENUP", "PENDOWN",
                    "SETXY", "REPEAT", "RANDOM", "SETHEADING"};
VecStr shortNames = {"FD", "BD", "RT", "LT", "PU", "PD",
                     "SETXY", "REPEAT", "RANDOM", "SH"};

std::string mainInstructions;
VecStr mainSet;
std::vector<pair2f> path;
std::vector<std::vector<pair2f>> paths;

struct Turtle {
    float x;
    float y;
    float angle;
    bool isDown;
    bool savePath;
    void fd(float value) {
        x += value * std::cos(angle*DEG2RAD);
        y += value * std::sin(angle*DEG2RAD);
        if (isDown && savePath) {
            path.push_back({x, y});
        }
    }
    void bd(float value) {
        return fd(-value);
    }
    void lt(float value) {
        angle += value;
        while (angle<0) {angle += 360;}
        while (angle>360) {angle -= 360;}
    }
    void rt(float value) {
        return lt(-value);
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
    void setxy(float x_, float y_) {
        x = x_;
        y = y_;
        if (isDown && savePath) {
            path.push_back({x, y});
        }
    }
    void set_heading(float ngl) {
        angle = ngl;
        while (angle<0) {angle += 360;}
        while (angle>360) {angle -= 360;}
    }
} mainT;

struct Function {
    VecStr parameters;
    VecStr funcInstructions;
    VecStr get_instructions(VecStr arguments) {
        VecStr newInstructions = funcInstructions;
        for (std::size_t i=0; i<arguments.size(); i++) {
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
        for (auto i : funcInstructions)
            std::cout << i << " ";
        std::cout << std::endl;
    }
};
std::map<std::string, Function> functions;

void init() {
    glClearColor(0.2, 0.3, 0.4, 1.0);
    srand(time(0));
    reset();
}

bool is_number(std::string s) {
    // test try {to_number} catch (...) {return false;} return true;
    bool isFloat(false);
    for (size_t i=0; i<s.length(); i++) {
        if (s[i]<'0' || s[i]>'9') {
            if (i==0 && s[i]=='-') { // for negative
                continue;
            } else if ((s[i]=='.' || s[i]==',') && !isFloat) { // for floats
                isFloat = true;
                continue;
            }
            return false;
        }
    }
    return true;
}

float to_number(std::string s) {
    bool isFloat(false);
    for (size_t i=0; i<s.size(); i++) {
        if (s[i]<'0' || s[i]>'9') {
            if (i==0 && s[i]=='-') { // for negative
                continue;
            } else if ((s[i]=='.' || s[i]==',') && !isFloat) { // for floats
                isFloat = true;
                continue;
            }
            throw std::invalid_argument("to_number");
        }
    }
    return std::stof(s);
}

int random_range(int a, int b) {
    if (a==b) {
        return a;
    } else if (a > b) {
        int tmp(b);
        b = a;
        a = tmp;
    }
    return a + rand()%(b-a);
}



void to_mainSet() {
    std::string s;
    //bool nextLineIsEndOfArg(false);
    for (size_t i=0; i<mainInstructions.length(); i++) {
        if (mainInstructions[i] != ' ' && mainInstructions[i] != '\n') {
            if (mainInstructions[i] == '[') {
                mainSet.push_back("[");
            } else if (mainInstructions[i] == ']') {
                if (s!="" && s!=" ")
                    mainSet.push_back(s);
                mainSet.push_back("]");
                s.clear();
            } else if (mainInstructions[i] == ':') {
                s += mainInstructions[i];
            } else {
                s += mainInstructions[i];
            }
        } else {
            if (s!="") {
                mainSet.push_back(s);
                if (s==" ") {
                    std::cout << "eee\n";
                }
            }
            s.clear();
        }
    }
    if (s!="") { // don't remember why I wrote this
        std::cout << s << std::endl;
        mainSet.push_back(s);
    }
    // replace the long words with abreviations
    for (size_t i=0; i<longNames.size(); i++) {
            std::replace(mainSet.begin(), mainSet.end(), longNames[i], shortNames[i]);
    }
}

bool extract_functions(VecStr set) {
    for (size_t i=0; i<set.size(); i++) {
        if (set[i] == "TO") {
            try {
                Function func;
                int toSkip(2);
                if (set[i+2][0] == ':') { // if the function takes arguments
                    while (set[i+toSkip][0] == ':') { // while it is an argument
                        func.parameters.push_back(set[i+toSkip]);
                        toSkip++;
                    }
                } else { // if the functions doesn't takes arguments
                    //toSkip--;
                }
                // first item after arguments
                while (set[i+toSkip] != "END") {
                    if (set[i+toSkip] == "TO") {
                        throw InvalidFunctionDefinition(set[i+1]);
                    }
                    func.funcInstructions.push_back(set[i+toSkip]);
                    toSkip++;
                    if (i+toSkip >= set.size()) {
                        throw InvalidFunctionDefinition(set[i+1]);
                    }
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
    size_t i; // to use it after if exception
    try {
        for (i=0; i<set.size()-1; i++) {
            int rndCount(0);
            while (set[i+1+rndCount] == "RANDOM") { // process all following randoms
            //if (set[i+1] == "RANDOM") {
                if (is_number(set[i+2+rndCount]) && is_number(set[i+3+rndCount])) {
                    float f = random_range(to_number(set[i+2+rndCount]), to_number(set[i+3+rndCount]));
                    set[i+1+rndCount] = std::to_string(f);
                    set.erase(set.begin()+i+2+rndCount);
                    set.erase(set.begin()+i+2+rndCount); // same index because the end is offset
                } else if (is_number(set[i+2+rndCount]) && !is_number(set[i+3+rndCount])) {
                    float f = random_range(0, to_number(set[i+2+rndCount]));
                    set[i+1+rndCount] = std::to_string(f);
                    set.erase(set.begin()+i+2+rndCount);
                } else {
                    std::cout << "nope\n";
                    //throw 
                }
                rndCount++;
                //std::cout << rndCount << set[i+1+rndCount] << std::endl;
            }
            // apply arithmetics after randoms are generated
            if (set[i] == "FD") {
                T.fd(to_number(set[i+1])); i++; // to skip the next item
            } else if (set[i] == "BD") {
                T.bd(to_number(set[i+1])); i++;
            } else if (set[i] == "RT") {
                T.rt(to_number(set[i+1])); i++;
            } else if (set[i] == "LT") {
                T.lt(to_number(set[i+1])); i++;
            } else if (set[i] == "REPEAT") {
                int count(-1); // to count '[]' in case they are nested
                int toSkip(0); // items to skip depending on the length
                try {
                    if (set[i+2]!="[") {
                        throw BracketNotMatching(1);
                    }
                    for (size_t j=i+3; j<set.size(); j++) {
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
                    if (count!=0) {
                        throw BracketNotMatching(count);
                    }
                } catch (BracketNotMatching& bnm) {
                    std::cerr << "Error (" << bnm.what() << ") >> ";
                    std::cerr << set[i] << " " << set[i+1] << " " << set[i+2] << std::endl;
                    return false;
                }
                // the +3 is to skip the "repeat N ["
                VecStr subset(set.begin()+i+3, set.begin()+i+3+toSkip);
                for (int x=0; x<(int)to_number(set[i+1]); x++) {
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
                T.setxy(to_number(set[i+1]), to_number(set[i+2]));
                i += 2;
            } else if (set[i] == "TO") { // skips the procedure definition
                size_t j(i);
                while (set[j] != "END") {
                    if (++j >= set.size()) {
                        throw InvalidFunctionDefinition(set[i+1]);
                    }
                }
                i = j;
                continue;
                
            } else if (functions.find(set[i]) != functions.end()) { // if the procedure is known
                auto f = functions.find(set[i]);
                VecStr args;
                try {
                    for (size_t arg=0; arg<f->second.parameters.size(); arg++) {
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
            } else if (set[i] == "SH") { // set heading
                int val;
                if (set[i+1] == "N") {val = 90;}
                else if (set[i+1] == "S") {val = -90;}
                else if (set[i+1] == "E") {val = 0;}
                else if (set[i+1] == "W") {val = 180;}
                else {val = to_number(set[i+1]);}
                T.set_heading(val); i++;
            } else if (set[i] != "]" && set[i] != "[" && set[i] != " " && !is_number(set[i])) {
                std::vector<char> buffer(set[i].begin(), set[i].end());
                throw UnknownName(&buffer[0]);
            }
        }
    } catch (const std::invalid_argument& ia) {
        std::cerr << "Error (Invalid argument to " << ia.what() << ") >> ";
        std::cerr << set[i] << " " << set[i+1] << " " << set[i+2] << std::endl;
        return false;
        
    } catch (UnknownName& un) {
        std::cerr << "Error (" << un.what() << " " << set[i] << ") >> ";
        for (int x=0; x<3; x++) {
            std::cerr << set[i+x] << " ";
        }
        std::cerr << std::endl;
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
    mainInstructions = fparse::get_file_instructions();
    to_mainSet();
    // in case there is an exception at EOF
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
        for (size_t i=0; i<p.size(); i++) {
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
    if (fparse::check_change(mainInstructions)) {
        reset();
    }
    glutTimerFunc(1000/FPS, timer_callback, 0);
}

void keyboard_callback(unsigned char key, int x, int y) {
    if (key==' ') {
        reset();
    }
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
    glutKeyboardFunc(keyboard_callback);
    init();
    
    glutMainLoop();
    return 0;
}
