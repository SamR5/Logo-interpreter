# Logo (language) interpreter

This is an interpreter of the Logo programming language  
Enter your commands in the "instructions.txt" file, save and run the program  
Pressing 'space' will reload the file and redraw the instructions  

## List of instructions
 - 'FORWARD N' or 'FD N' : the turtle goes forward for 'N' pixels
 - 'BACKWARD N' or 'BD N' : the turtle goes backward for 'N' pixels
 - 'RIGHT N' or 'RT N' : the turtle turn right for 'N' degrees
 - 'LEFT N' or 'LT N': the turtle turn left for 'N' degrees
 - 'REPEAT N []' : repeat the instructions between brackets 'N' times
 - 'PENUP PENDOWN' or 'PU PD' : for pen up and pen down. Lift/drop the pen to stop drawing
 - 'SETXY X Y' : go to the position (x, y). The screen is centered to (0, 0)
 - 'TO PROCEDURENAME :ARG1 ARG2... \n INSTRUCTIONS \n END' : customizable function
 - '#' : comments, everything until newline don't affect the program
 - '#*   *#' multiline comment

## Examples ##
 - square > REPEAT 4 [FD 100 RT 90]
 - circle > REPEAT 360 [FD 2 RT 1] or REPEAT R [FD A RT B] with R*B=360
 - shift to the left > PU LT 90 FD 100 RT 90 PD
 - dotted line > REPEAT 40 [FD 2 PU FD 2 PD]
 - dotted square > REPEAT 4 [REPEAT 40 [FD 2 PU FD 2 PD] RT 90]
 - function triangle > TRIANGLE :L REPEAT 3 [FD L RT 120] END


