#include "libTimer.h"
#include "buzzer.h"
#include <msp430.h>

static int counter = 0; //Counter to keep track of the notes

void buzzer_init(){
    
  timerAUpmode(); //used to drive speaker
  P2SEL2 &= ~(BIT6 | BIT7);
  P2SEL &= ~BIT7;
  P2SEL |= BIT6;
  P2DIR = BIT6; //enable output to speaker (P2.6)
}

/*
  pianodSong produce the sound of the frequency specified on
  each case.
*/
void pianoSong(){
  switch(counter){
  case 0: buzzer_set_period(850); counter++; break; //Lower C note
  case 1:
  case 4: buzzer_set_period(530); counter++; break; //G note
  case 2:
  case 7:
  case 6: buzzer_set_period(610); counter++; break; //F note
  case 9:
  case 5: buzzer_set_period(650); counter++; break; //E note
  case 8: buzzer_set_period(840); counter++; break;//D note
  case 3: buzzer_set_period(375); counter++; break; //C note
  }
}

void buzzer_set_period(short cycles){
  CCR0 = cycles;
  CCR1 = cycles >> 1; //one half cycle
} 
