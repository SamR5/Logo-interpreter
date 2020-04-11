# Logo (language) interpreter

This is an interpreter of the Logo programming language  
Enter your commands in the "instructions.txt" file, save and run the program  
Pressing 'space' will reload the file and redraw the instructions  

## List of instructions

 - Movements

        FORWARD N   # goes forward by N pixels, can be shorten to FD N
        BACKWARD N  # goes backward by N pixels, can be shorten to BD N
        SETXY X Y   # move the turtle to the position (X, Y)
 - Rotations

        RIGHT A    # turn right by A degrees, can be shorten to RT A
        LEFT -A    # turn left by -A degrees, can be shorten to LT -A
 - Loops

        REPEAT N [FD 10 RT 60]  # repeat the instructions between brackets N times
 - States

        PENUP   # stop recording the path (don't erase the previous), can be shorten to PU
        PENDOWN # start recording the path, can be shorten to DP
 - Other

        # one line comment, everything until newline don't affect the program
        #*
        multiline comment
        *#
 - Procedures

        TO DOTTED_LINE              # 'TO' and the name of the procedure  
        REPEAT 40 [FD 2 PU FD 2 PD] # instructions  
        END                         # end of the procedure
        DOTTED_LINE                 # a call of the procedure
        
        TO TRIANGLE :L          # this one has an argument (they must be space separated)
        REPEAT 3 [FD L RT 120]  
        END                     
        TRIANGLE 30             # will execute: REPEAT 3 [FD 30 RT 120]
 - Not implemented yet

        RANDOM N            # a random integer between 0 and N (included)
        RANDOM A B          # a random integer between A and B (included)
        SETHEADING X        # set the heading to X (counterclockwise with 0 to the right)

## TODO
 - A set heading keyword with an angle or NSEW
 - A random keyword

## Updates
### 11/04/2020
    - Exception handling
    - Automatically draw when the file is modified
    - Allow lowercase (the whole instructions are case insensitive)
