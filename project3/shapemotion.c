#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include <string.h>
#include "buzzer.h"

#define GREEN_LED BIT6

AbRect rect = {abRectGetBounds, abRectCheck, {2,10}};

u_char player1Score = '0';
u_char player2Score = '0';
static int state = 0;

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2-5, screenHeight/2-15}
};

Layer fieldLayer = { /* this layer acts as a field */
  (AbShape *)&fieldOutline,
  {screenWidth/2, screenHeight/2},
  {0,0}, {0,0},
  COLOR_BLACK,
  0
};

Layer layer3 = {		/**< Layer with an white circle */
  (AbShape *)&circle4,
  {(screenWidth/2), (screenHeight/2)}, /**<center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_YELLOW,
  &fieldLayer,
};

Layer layer1 = {		/**< Layer with a blue rect */
  (AbShape *) &rect,
  {screenWidth/2-50, screenHeight/2+5},     //current pos
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_RED,
  &layer3
};

Layer layer2 = {		/**< Layer with a blue rect */
  (AbShape *)&rect,
  {screenWidth/2+50, screenHeight/2+5}, //current pos
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_BLUE,
  &layer1,
};

typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */
MovLayer ml1 = { &layer1, {0,3}, 0 }; //Red rectangle(paddle)
MovLayer ml2 = { &layer2, {0,3}, 0 }; //Blue rectangle(padle)
MovLayer ml3 = { &layer3, {2,4}, 0 }; //White circle as a Ball


void movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */ 
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { // for each moving layer
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */

  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { // for each moving layer 
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
    		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer; 
	     probeLayer = probeLayer->next) {
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break; 
	   }
	}
	lcd_writeColor(color); 
      } // for col
    } // for row
  } // for moving layer being updated
}	  

/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */

void moveUp(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; ml; ml = ml->next) {
    vec2Sub(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 1; axis < 2; axis ++) {
		// this if statement handles when a collision happens in the fence
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
	int velocity = ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }	
    } /**< for axis */
    ml->layer->posNext = newPos;
  } /**< for ml */
}

void moveDown(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 1; axis < 2; axis ++) {
		// this if statement handles when a collision happens in the fence
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
	int velocity = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }	// this if statement handles when a collision happens in the fence
    } /**< for axis */
    ml->layer->posNext = newPos;
  } /**< for ml */
}

void moveBall(MovLayer *ml, Region *fence1, MovLayer *ml2, MovLayer *ml3)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  int velocity;
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++){
      if((shapeBoundary.topLeft.axes[axis] < fence1->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence1->botRight.axes[axis])){
	velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }
      else if((abShapeCheck(ml2->layer->abShape, &ml2->layer->posNext, &ml->layer->posNext))){
	velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }
      else if((abShapeCheck(ml3->layer->abShape, &ml3->layer->posNext, &ml->layer->posNext))){
	velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }
      else if((shapeBoundary.topLeft.axes[0] < fence1->topLeft.axes[0])){
	newPos.axes[0] = screenWidth/2;
	newPos.axes[1] = screenHeight/2;
	player2Score = player2Score - 255;
      }
      else if((shapeBoundary.botRight.axes[0] > fence1->botRight.axes[0])){
	newPos.axes[0] = screenWidth/2;
	newPos.axes[1] = screenHeight/2;
	player1Score = player1Score - 255;
      }
      if(player1Score == '5' || player2Score == '5'){
	state = 1;
      }
    } /**< for axis */
    ml->layer->posNext = newPos;
  } /**< for ml */
}

u_int bgColor = COLOR_WHITE;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */
Region fieldFence;		/**< fence around playing field  */


/** Watchdog timer interrupt handler.*/
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  u_int switches = p2sw_read();
  
  if(count == 10){                /** 15 Interrups per second*/
    switch(state){
		
    case 0:
      moveBall(&ml3, &fieldFence, &ml1, &ml2);
      break;
	  
    case 1:
      layerDraw(&layer2);
	  
      if(player1Score > player2Score)
	drawString5x7(28, 50, "Red Wins!", COLOR_RED, COLOR_WHITE);

      else if(player1Score < player2Score)
	drawString5x7(28, 50, "Blue Wins!", COLOR_BLUE, COLOR_WHITE);

      break;
    }

	pianoSong2();
    
    if(switches & (1<<3)){
      moveUp(&ml2, &fieldFence);
    }
    if(switches & (1<<2)){
      moveDown(&ml2, &fieldFence);
    }
    if(switches & (1<<1)){
      moveUp(&ml1, &fieldFence);
    }
    if(switches & (1<<0)){
      moveDown(&ml1, &fieldFence);
    }
    redrawScreen = 1;
    count = 0;
  }
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}

/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED;		//Green led on when CPU on		
  P1OUT |= GREEN_LED;

  configureClocks(); //initialize clocks
  lcd_init(); //initialize lcd
  p2sw_init(15); //initialize switches
  layerInit(&layer2); //Passes the first element from a MoveLayer LL to initilize shapes
  layerDraw(&layer2); //Passes the first element from a MoveLayer LL to draw shapes
  layerGetBounds(&fieldLayer, &fieldFence);
  buzzer_init();
  enableWDTInterrupts();      // enable periodic interrupt 
  or_sr(0x8);	              // GIE (enable interrupts) 

  u_int switches = p2sw_read();
  
  for(;;){ 
  
    while (!redrawScreen) { // Pause CPU if screen doesn't need updating 
      P1OUT &= ~GREEN_LED; // Green led off witHo CPU 
      or_sr(0x10); //< CPU OFF
    }
	
    P1OUT |= GREEN_LED; // Green led on when CPU on
    redrawScreen = 0;
    movLayerDraw(&ml3, &layer2);
    movLayerDraw(&ml2, &layer2);
    movLayerDraw(&ml1, &layer2);
	
    drawChar5x7(23, 5, player1Score, COLOR_BLACK, COLOR_WHITE); //Scoreboard 
    drawChar5x7(95, 5, player2Score, COLOR_BLACK, COLOR_WHITE); //Scoreboard
    drawString5x7(40, 5, "-SCORE-", COLOR_BLACK, COLOR_WHITE); // Score
	drawString5x7(5, 150, "Red", COLOR_RED, COLOR_WHITE);
	drawString5x7(90, 150, "Blue", COLOR_BLUE, COLOR_WHITE);
  }
}
