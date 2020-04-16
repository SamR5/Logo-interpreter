/*

LOGO programming language

Parse the instructions in instructions.txt and draw

*/

#include <GL/gl.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <utility>
#include <map>
#include <algorithm>
#include <ctime>
#include <random>
#include "custom_exception.h"
#include "file_parse.h"

#define WIDTH 800
#define HEIGHT 600
#define FPS 40
#define PI 3.1415926
#define DEG2RAD PI/180.0

typedef std::pair<float, float> pair2f;
typedef std::pair<int, int> pair2i;
typedef std::vector<std::string> VecStr;

struct Turtle;
struct Function;


void init();
bool is_number(const std::string& s);
float to_number(const std::string& s);
int random_range(int a, int b);
void to_mainSet();
bool extract_functions(const VecStr& set);
int apply_operator(VecStr& set, const char op);
int apply_arithmetics(VecStr& set);
bool move_turtle(VecStr set, Turtle& T); // return false on exception
void reset();
// -1 is for initialization, 0 resets all lines to 0 and 1 for all lines drawn
void reset_draw_state(int state);
void delete_paths(size_t i, size_t j); // in case of keyword cs
float distance(float dx, float dy);
const pair2f draw_line(float x1, float y1, float x2, float y2, float ratio);

void display_callback();
void reshape_callback(int width, int height);
void timer_callback(int);
void keyboard_callback(unsigned char key, int x, int y);

const VecStr longNames = {"FORWARD", "BACKWARD", "RIGHT", "LEFT", "PENUP", "PENDOWN",
                          "SETXY", "REPEAT", "RANDOM", "SETHEADING", "CLEARSCREEN"};
const VecStr shortNames = {"FD", "BD", "RT", "LT", "PU", "PD",
                           "SETXY", "REPEAT", "RANDOM", "SH", "CS"};
const char* operators = "+-*/";
const char* brackets = "[]()";
std::string mainInstructions;
VecStr mainSet;
std::vector<pair2f> path;
std::vector<std::vector<pair2f>> paths;
// stores the state of each line of each path 0 is not drawn, 1 is complete line
// and the values in between are the ratios of the line drawn (to simulate movement)
std::vector<std::vector<float>> toDraw;
// true to display and false to skip
// used for clearscreen command to hide everything past
std::vector<pair2i> cls;
float speed; // speed of the turtle in pixels per frame

struct Turtle {
    float x;
    float y;
    float angle;
    bool isDown;
    bool savePath;
    void fd(const float& value) {
        x += value * std::cos(angle*DEG2RAD);
        y += value * std::sin(angle*DEG2RAD);
        if (isDown && savePath) {
            path.push_back({x, y});
        }
    }
    void bd(const float& value) {
        return fd(-value);
    }
    void lt(const float& value) {
        angle += value;
        while (angle<0) {angle += 360;}
        while (angle>360) {angle -= 360;}
    }
    void rt(const float& value) {
        return lt(-value);
    }
    void up() {
        if (!isDown) // bug with one point path
            return;
        isDown = false;
        if (savePath) {
            paths.push_back(path);
            path.clear();
        }
    }
    void down() {
        if (isDown) // bug with one point path
            return;
        isDown = true;
        if (savePath) {
            path.push_back({x, y});
        }
    }
    void setxy(const float& x_, const float& y_) {
        x = x_;
        y = y_;
        if (isDown && savePath) {
            path.push_back({x, y});
        }
    }
    void set_heading(const float& ngl) {
        angle = ngl;
        while (angle<0) {angle += 360;}
        while (angle>360) {angle -= 360;}
    }
    void clear_screen() const {
        if (savePath) {
            cls.push_back({paths.size(), path.size()-1});
        }
    }
} mainT;

struct Function {
    VecStr parameters;
    VecStr funcInstructions;
    const VecStr get_instructions(const VecStr& arguments) const {
        VecStr newInstructions = funcInstructions;
        for (std::size_t i=0; i<arguments.size(); i++) {
            std::replace(newInstructions.begin(), newInstructions.end(),
                         parameters[i], arguments[i]);
        }
        return newInstructions;
    }
    bool is_valid(const VecStr& arguments) const {
        const VecStr newInst = get_instructions(arguments);
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
        std::cout << "\ninstructions: ";
        for (auto i : funcInstructions)
            std::cout << i << " ";
        std::cout << std::endl;
    }
};
std::map<const std::string, Function> functions;

void init() {
    glClearColor(0.2, 0.3, 0.4, 1.0);
    srand(time(0));
    reset();
    reset_draw_state(-1);
    speed = 2;
}

bool is_number(const std::string& s) {
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

float to_number(const std::string& s) {
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
    for (size_t i=0; i<mainInstructions.length(); i++) {
        if (mainInstructions[i] != ' ' && mainInstructions[i] != '\n') {
            // if there is a bracket, separate it from the other
            if (*std::find(brackets, brackets+4, mainInstructions[i]) != '\0') {
                if (s!="" && s!=" ")
                    mainSet.push_back(s);
                s.clear();
                s += mainInstructions[i];
                mainSet.push_back(s);
                s.clear();
            } else if (mainInstructions[i] == ':') { // the end of arguments will be known after '\n'
                s += mainInstructions[i];
            } // if there is an operator sign, separate it from others
            else if (*std::find(operators, operators+4, mainInstructions[i]) != '\0') {
                if ((is_number(s) && s!="") || s[0]==':') { // if number or the parameter of a function
                    if (s!="")
                        mainSet.push_back(s);
                    s.clear();
                    s += mainInstructions[i];
                    mainSet.push_back(s);
                    s.clear();
                } // in case there is A*-1 or A+-2
                else if (*std::find(operators, operators+4, mainSet.back().back()) != '\0' ||
                         *std::find(brackets, brackets+4, mainSet.back().back()) != '\0') {
                    if (mainInstructions[i]=='-') {
                        s += mainInstructions[i];
                    } else if (s!="") {
                        std::cout << s << "e\n";
                        mainSet.push_back(s);
                        s.clear();
                        s += mainInstructions[i];
                    } else {
                        s += mainInstructions[i];
                        mainSet.push_back(s);
                        s.clear();
                    }
                } else {
                    s += mainInstructions[i];
                }
            }
            else {
                s += mainInstructions[i];
            }
        } else {
            if (s!="" && s!=" ") {
                mainSet.push_back(s);
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

bool extract_functions(const VecStr& set) {
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

int apply_operator(VecStr& set, const char op) {
    bool change(false);
    for (int i=0; i<set.size()-2; i++) {
        if (is_number(set[i]) && is_number(set[i+2])) {
            if (set[i+1].size()==1 && set[i+1][0]==op) {
                if (op=='/' && to_number(set[i+2]) == 0)
                    return i;
                switch (op) {
                    case '/': set[i] = std::to_string(to_number(set[i])/to_number(set[i+2])); break;
                    case '*': set[i] = std::to_string(to_number(set[i])*to_number(set[i+2])); break;
                    case '-': set[i] = std::to_string(to_number(set[i])-to_number(set[i+2])); break;
                    case '+': set[i] = std::to_string(to_number(set[i])+to_number(set[i+2])); break;
                }
                set.erase(set.begin()+i+2); // erase the second number
                set.erase(set.begin()+i+1); // erase the operator
                change = true;
            }
        }
    }
    if (change) {
        return -1;
    }
    return -2;
}

int apply_arithmetics(VecStr& set) {
    bool change(false);
    for (int i=0; i<set.size()-2; i++) {
        if (set[i]=="(" && set[i+2]==")") {
            set.erase(set.begin()+i+2); // erase the second parenthesis
            set.erase(set.begin()+i); // erase the first parenthesis
            change=true;
        }
    }
    if (change) {
        return apply_arithmetics(set);
    }
    
    int error(-1); // -2 = changes happened, -1 = no change and >0 is the index of the error
    const char* oper = "/*-+";
    int current;
    for (int i=0; i<4; i++) {
        current = i;
        while (error==-1) {
            error = apply_operator(set, oper[i]);
            if (error==-1) {
                change = true;
                i = current-1; // will add 1 next loop
            }
        }
        if (error!=-2)
            return error;
        error = -1;
    }
    return -1;
}

bool move_turtle(VecStr set, Turtle& T) {
    size_t i; // to use it after in case of exception
    try {
        for (i=0; i<set.size()-1; i++) {
            int rndCount(0);
            while (set[i+1+rndCount] == "RANDOM") { // process all following randoms
                if (is_number(set[i+2+rndCount]) && is_number(set[i+3+rndCount])) {
                    float f = random_range(to_number(set[i+2+rndCount]),
                                           to_number(set[i+3+rndCount]));
                    set[i+1+rndCount] = std::to_string(f);
                    set.erase(set.begin()+i+3+rndCount); // erase second arg
                    set.erase(set.begin()+i+2+rndCount); // erase first arg
                } else if (is_number(set[i+2+rndCount]) && !is_number(set[i+3+rndCount])) {
                    float f = random_range(0, to_number(set[i+2+rndCount]));
                    set[i+1+rndCount] = std::to_string(f);
                    set.erase(set.begin()+i+2+rndCount); // erase first arg
                } else {
                    std::cout << "nope\n";
                    //throw 
                }
                rndCount++;
            }
            int result = apply_arithmetics(set);
            if (result!=-1) { // return -1 when ok otherwise the index of the problem
                i += result;
                throw std::invalid_argument("arithmetics");
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
            } else if (set[i] == "SH") { // set heading
                int val;
                if (set[i+1] == "N") {val = 90;}
                else if (set[i+1] == "S") {val = -90;}
                else if (set[i+1] == "E") {val = 0;}
                else if (set[i+1] == "W") {val = 180;}
                else {val = to_number(set[i+1]);}
                T.set_heading(val); i++;
            } else if (set[i] == "CS") {
                T.clear_screen();
            } else if (set[i] != "]" && set[i] != "[" && set[i] != " " && !is_number(set[i])) {
                std::vector<char> buffer(set[i].begin(), set[i].end());
                throw UnknownName(&buffer[0]);
            } else if ((set[i] == "[" && is_number(set[i+1])) ||
                       (set[i] == "]" && is_number(set[i+1]))) {
                throw UselessNumber();
            } else {
                
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
    } catch (UselessNumber& un) {
        std::cerr << "Error (" << un.what() << ") >> ";
        std::cerr << set[i] << " " << set[i+1] << " " << set[i+2] << std::endl;
        return false;
    } catch (std::logic_error& le) {
        std::cerr << le.what() << std::endl << i << std::endl;
        for (auto j : set) {
            std::cerr << j << '-';
        } std::cerr << std::endl;
        return false;
    } catch (BracketNotMatching& bnm) {
        std::cerr << "Error (" << bnm.what() << ") >> ";
        std::cerr << set[i] << " " << set[i+1] << " " << set[i+2] << std::endl;
        return false;
    } catch (InvalidArgToFunction& iatf) {
        std::cerr << "Error (" << iatf.what() << ") >> " << iatf.get_name() << std::endl;
        std::cerr << "Parameters: " << iatf.get_param_str() << std::endl;
        std::cerr << "Arguments: " << iatf.get_args_str() << std::endl;
        return false;
    }
    return true;
}

void reset() {
    std::cout << "New drawing\n";
    mainSet.clear();
    path.clear();
    paths.clear();
    cls.clear();
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

void reset_draw_state(int state) {
    for (size_t p=0; p<paths.size(); p++) {
        if (state==-1) {
            toDraw.push_back({});
        }
        for (size_t q=0; q<paths[p].size()-1; q++) {
            if (state==-1) {
                toDraw[p].push_back(0);
            } else {
                toDraw[p][q] = state;
            }
        }
    }
}

void delete_paths(size_t i, size_t j) {
    if (i!=0) {
        for (size_t ii=0; ii<i; ii++) {
            for (size_t jj=0; jj<toDraw[ii].size(); jj++) {
                toDraw[ii][jj] = -1;
            }
        }
        return delete_paths(i-1, j);
    }
    for (size_t jj=0; jj<j; jj++) {
        toDraw[0][jj] = -1;
    }
}

float distance(float dx, float dy) {
    return std::sqrt(dx*dx + dy*dy);
}

const pair2f draw_line(float x1, float y1, float x2, float y2, float ratio) {
    const float dx = x2-x1;
    const float dy = y2-y1;
    const float dist = distance(dx, dy);
    float xt, yt; // intermediate value
    const float toTravel = speed;
    if (ratio >= 1) {
        glBegin(GL_LINE_STRIP);
          glVertex2f(x1, y1);
          glVertex2f(x2, y2);
        glEnd();
        return {0, dist};
    } else {
        xt = x1+dx*std::min(ratio+toTravel/dist, 1.0f);
        yt = y1+dy*std::min(ratio+toTravel/dist, 1.0f);
        glBegin(GL_LINE_STRIP);
          glVertex2f(x1, y1);
          glVertex2f(xt, yt);
        glEnd();
    }
    // distance to new point - distance already traveled = distance traveled
    // return total distance traveled, total distance
    return {distance(xt-x1, yt-y1) - dist*ratio, dist};
}

void display_callback() {
    glClear(GL_COLOR_BUFFER_BIT); // clear screen
    // red cross on the center
    glColor3f(1, 0, 0);
    glBegin(GL_LINES);
      glVertex2f(-6, 0); glVertex2f(5, 0);
      glVertex2f(0, -5); glVertex2f(0, 6);
    glEnd();
    
    pair2i c;
    glColor3f(1, 1, 1);
    pair2f p1, p2; // points
    float d(0); // distance traveled
    pair2f temp; // to store (dist traveled, total dist) returned by draw_line
    // 1 call of the funtion per frame so the distance to travel is the speed (in pix/frame)
    float toTravel(speed);
    
    for (size_t i=0; i<toDraw.size(); i++) {
        for (size_t j=0; j<toDraw[i].size(); j++) {
            if (cls.size()>0) {
                c = cls.front();
                if (c.first==i && c.second==j) {
                    delete_paths(i, j);
                    cls.erase(cls.begin());
                }
            }
            p1 = paths[i][j];
            p2 = paths[i][j+1];
            if (p1==p2) { // bug inf loop if same points
                continue;
            }
            if (toDraw[i][j] >= 1) { // draw the whole line
                draw_line(p1.first, p1.second, p2.first, p2.second, 1);
            } else if (toDraw[i][j] < 0) {
                continue;
            } else {
                temp = draw_line(p1.first, p1.second, p2.first, p2.second, toDraw[i][j]);
                d += temp.first;
                toDraw[i][j] += temp.first/temp.second; // ratio traveled/total
                if (d<toTravel) {
                    j--; // will recheck the current in case not fully drawn
                    continue;
                } else {
                    glutSwapBuffers();
                    return;
                }
            }
        }
    }
    glutSwapBuffers();
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
        toDraw.clear();
        reset_draw_state(-1);
    }
    glutTimerFunc(1000/FPS, timer_callback, 0);
}

void keyboard_callback(unsigned char key, int x, int y) {
    if (key==' ') {
        reset();
        reset_draw_state(0);
    } else if (key==13) {
        reset();
        reset_draw_state(1);
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
