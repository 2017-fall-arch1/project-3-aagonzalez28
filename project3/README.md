This directory contains:

code that implements a game
a demo program that uses it

This demo contains the following files: buzzer.h: header file of llist structure & "public" interface functions shapemotiondemo.c:

To compile:

$ make
To test it, try:

$ make demo
To delete binaries:

$ make clean

The Layer fieldLayer is the outline of a rectangle that works as a border of the field. Layer 1,2, and 3 are the two rectangles and the circle. To win in this game the player that reaches 8 points wins.

The following methods create the fields:

AbRectOutline fieldOutline = { abRectOutlineGetBounds, abRectOutlineCheck,
{screenWidth/2-5, screenHeight/2-15} };

Layer fieldLayer = { (AbShape *)&fieldOutline, {screenWidth/2, screenHeight/2}, {0,0}, {0,0}, COLOR_GREEN, 0 };

References: 

Dr. Eric Freudenthal code git hub http://stackoverflow.com/questions/29529476/c-print-ping-pong-using-semaphores-and-threads
