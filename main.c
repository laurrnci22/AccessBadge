#include <avr/io.h>

#define LED     PD5
#define BOUTON  PB5


// PD => DDRD ou PORTD
// PB => DDRB ou PORTB
// -vvv : pour voir plus d'informations

int main(void){
  DDRD |= (1 << LED);         // Sortie pour la LED
  DDRB &= ~(1 << BOUTON);     // Entrée pour le bouton
  PORTB |= (1 << BOUTON);     // Activation de la résistance de tirage interne pour le bouton

  while(1){
      if(PINB & (1 << BOUTON)) 
          PORTD &= ~(1 << LED);  // LED éteinte si bouton relâché
      else 
          PORTD |= (1 << LED);   // LED allumée si bouton appuyé
  }
}