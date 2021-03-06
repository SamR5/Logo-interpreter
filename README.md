# Logo (language) interpreter

This is an interpreter of the Logo programming language  
Enter your commands in the "instructions.txt" file, save and run the program  
Pressing 'space' will reload the file and redraw the instructions  

## List of instructions
##### *The instructions here are in uppercase but can be lowercase, the text is case insensitive.*  
 - Movements

        FORWARD N   # goes forward N pixels, can be shorten to FD N
        BACKWARD N  # goes backward N pixels, can be shorten to BD N
        SETXY X Y   # move the turtle to the position (X, Y)
 - Rotations

        RIGHT A         # turn right A degrees, can be shorten to RT A
        LEFT A          # turn left A degrees, can be shorten to LT A
        SETHEADING X    # set heading to X degrees (anticlockwise with 0=east)
        SETHEADING N    # set heading to North (valid with NSEW)
 - Loops

        REPEAT N [FD 10 RT 60]  # repeat the instructions between brackets N times
 - States

        PENUP   # stop path recording path, can be shorten to PU
        PENDOWN # resume path recording, can be shorten to DP
 - Other

        RANDOM N            # a random integer between 0 and N (included)
        RANDOM A B          # a random integer between A and B (included)
        # one line comment, everything after don't affect the program
        #*
        multiline comment
        *#
        CLEARSCREEN or CS       # clear the screen
 - Procedures

        TO DOTTED_LINE              # 'TO' and the name of the procedure  
        REPEAT 40 [FD 2 PU FD 2 PD] # instructions  
        END                         # end of the procedure
        DOTTED_LINE                 # a call of the procedure
        
        TO TRIANGLE :L :A :B        # this procedure has three parameters
        REPEAT 3 [FD :L RT 120]     # parameters are prefixed with ":"
        END                     
        TRIANGLE 30                 # will execute: REPEAT 3 [FD 30 RT 120]
 - Not implemented yet

        SETPENCOLOR R G B       # change the pen color (3 integers <256)
        SETBGCOLOR R G B        # change the background color (3 integers <256)
        SETSPEED                # set the speed of the animation in pixels per frame (fixed at 40 FPS)
        WAIT N                  # wait N seconds

## TODO
 - A set pen color keyword with RGB as arguments
 - A wait keyword to control the animation

## Updates

### 15/04/2020
 - New keyword : CLEARSCREEN (or CS)

### 14/04/2020
 - Handle arithmetic operations +-*/()

### 13/04/2020
 - The drawing is animated
 - Press the spacebar to reset or enter to go to the end

### 12/04/2020
 - Custom exceptions in separated file
 - Every parameter of procedure should be prefixed with ":"
 - Supports floating point numbers
 - Handle bracket exception with "repeat"
 - New keyword : RANDOM
 - New keyword : SETHEADING (or SH)

### 11/04/2020
 - Exception handling
 - Automatically draw when the file is modified
 - Allow lowercase (the whole instructions are case insensitive)
