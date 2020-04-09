/*

LOGO programming language

Parse the instructions in instructions.txt and draw

TODO :
 - functions
 - multi line comments (#* ... *#)
 - clearscreen
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

#define WIDTH 800
#define HEIGHT 600
#define FPS 60
#define PI 3.1415926
#define DEG2RAD PI/180.0

typedef std::pair<float, float> pair2f;

void init();
bool is_number(std::string s);
std::string get_instructions();
void remove_lead_trail_space(std::string& s);
void remove_comment(std::string& s); // if comment at end of line
void to_tokens();
void move_turtle(std::vector<std::string> token);
void reset();

void display_callback();
void reshape_callback(int width, int height);
void timer_callback(int);
void keyboard_callback(unsigned char key, int x, int y);

std::string instructions;
std::vector<std::string> tokens;
std::vector<pair2f> path;
std::vector<std::vector<pair2f>> paths;
//std::map<std::string, std::vector<std::string>> functions;

struct Function {
    //std::string name;
    std::vector<std::string> args;
    std::vector<std::string> instructions;
    std::vector<std::string> get_instructions(std::vector<std::string> arguments) {
        std::vector<std::string> newInstructions = instructions;
        for (int i=0; i<arguments.size(); i++) {
            std::replace(newInstructions.begin(), newInstructions.end(), 
                         args[i], arguments[i]);
        }
        return newInstructions;
    }
    void show() {
        std::cout << "args: ";
        for (auto i : args)
            std::cout << i << " ";
        std::cout << std::endl;
        std::cout << "instructions: ";
        for (auto i : instructions)
            std::cout << i << " ";
        std::cout << std::endl;
    }
};
std::map<std::string, Function> functions;

struct Turtle {
    float x;
    float y;
    int angle;
    bool isUp;
    void fd(int value) {
        x += value * std::cos(angle*DEG2RAD);
        y += value * std::sin(angle*DEG2RAD);
        if (!isUp) {
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
        isUp = true;
        paths.push_back(path);
        path.clear();
    }
    void down() {
        isUp = false;
        path.clear();
        path.push_back({x, y});
    }
    void setxy(int x_, int y_) {
        x = x_;
        y = y_;
        if (!isUp) {
            path.push_back({x, y});
        }
    }
} T;

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

std::string get_instructions() {
    std::ifstream file("instructions.txt");
    std::stringstream ss;
    std::string temp;
    bool stopParse(false); // in case of multiline comment
    
    if (file) {
        while (getline(file, temp)) {
            if (stopParse || temp=="") { // getline doesn't catch the endline'
                continue;
            }
            remove_lead_trail_space(temp);
            if (temp[0]=='#' && temp[1]=='*') {
                stopParse = true;
                continue;
            } else if (temp[0]=='*' && temp[1]=='#') {
                stopParse = false;
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
    return result;
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

void to_tokens() {
    std::string s;
    bool nextLineIsEndOfArg(false);
    for (int i=0; i<instructions.length(); i++) {
        if (instructions[i] != ' ' && instructions[i] != '\n') {
            if (instructions[i] == '[') {
                tokens.push_back("[");
            } else if (instructions[i] == ']') {
                tokens.push_back(s);
                tokens.push_back("]");
                s.clear();
                i++;
            } else if (instructions[i] == ':') {
                tokens.push_back(":");
                nextLineIsEndOfArg = true;
            } else {
                s += instructions[i];
            }
        } else {
            tokens.push_back(s);
            s.clear();
            if (nextLineIsEndOfArg && instructions[i] == '\n') {
                tokens.push_back("END_OF_ARGS");
                nextLineIsEndOfArg = false;
            }
        }
    }
    if (s!="")
        tokens.push_back(s);
    /*
    for (auto i : tokens) {
        std::cout << i << '-';
    }
    std::cout << std::endl;
    */
}

void move_turtle(std::vector<std::string> token) {
    for (int i=0; i<token.size(); i++) {
        if (token[i] == "FD" || token[i] == "FORWARD") {
            T.fd(std::stoi(token[i+1]));
        } else if (token[i] == "BD" || token[i] == "BACKWARD") {
            T.bd(std::stoi(token[i+1]));
        } else if (token[i] == "RT" || token[i] == "RIGHT") {
            T.rt(std::stoi(token[i+1]));
        } else if (token[i] == "LT" || token[i] == "LEFT") {
            T.lt(std::stoi(token[i+1]));
        } else if (token[i] == "REPEAT") {
            int count(-1); // to count '[]' in case they are nested
            int toSkip(0); // tokens to skip depending on the length
            for (int j=i+3; j<token.size(); j++) {
                if (token[j] == "[") {
                    count--;
                } else if (token[j] == "]") {
                    count++;
                }
                toSkip++;
                if (count==0) {
                    break;
                }
            }
            std::vector<std::string> subtoken(token.begin()+i+3,
                                              token.begin()+i+3+toSkip);
            for (int x=0; x<std::stoi(token[i+1]); x++) {
                move_turtle(subtoken);
            }
            i += toSkip;
        } else if (token[i] == "PU" || token[i] == "PENUP") {
            T.up();
        } else if (token[i] == "PD" || token[i] == "PENDOWN") {
            T.down();
        } else if (token[i] == "SETXY") {
            T.setxy(std::stoi(token[i+1]), std::stoi(token[i+2]));
            i += 2;
        } else if (token[i] == "TO") { // add a function to the list
            Function func;// = {token[i+1]};
            int toSkip(2);
            if (token[i+2] == ":") { // if the function takes arguments
                toSkip++;
                while (token[i+toSkip] != "END_OF_ARGS") {
                    
                    func.args.push_back(token[i+toSkip]);
                    toSkip++;
                }
            }
            // first token after arguments
            toSkip++;
            while (token[i+toSkip] != "END") {
                func.instructions.push_back(token[i+toSkip]);
                toSkip++;
            }
            functions[token[i+1]] = func;
            i += toSkip;
            //f->second.show();
        } else if (functions.find(token[i]) != functions.end()) { // if the function is known
            auto f = functions.find(token[i]);
            std::vector<std::string> args;
            for (int arg=0; arg<f->second.args.size(); arg++) {
                args.push_back(token[i+1+arg]);
            }
            
            move_turtle(f->second.get_instructions(args));
        } else if (token[i] != "]" && token[i] != "[" && !is_number(token[i])) {
            std::cout << "Error : command not found " << token[i] << std::endl;
        }
    }
}

void reset() {
    tokens.clear();
    path.clear();
    paths.clear();
    T.x = T.y = 0;
    T.angle = 0;
    T.isUp = false;
    path.push_back({T.x, T.y});
    instructions = get_instructions();
    to_tokens();
    
    move_turtle(tokens);
    T.up(); // saves the last path
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
    glutTimerFunc(1000/FPS, timer_callback, 0);
}

void keyboard_callback(unsigned char key, int x, int y) {
    if (key == ' ') {
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
