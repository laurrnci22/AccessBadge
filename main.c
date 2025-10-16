#include <avr/io.h>
#include <util/delay.h>
#include "SSD1306.h"
#include "Font5x8.h"

#define LED4 PD4
#define LED5 PD5
#define LED6 PD6
#define LED7 PD7

#define BOUTON4 PB4
#define BOUTON5 PB5
#define BOUTON6 PB6

int main(void) {
  // appel des méthodes
  return 0;
}


/* Méthode permettant d'afficher des messages à l'écran */
int turnScreen(void) 
{
    GLCD_Setup();
    GLCD_SetFont(Font5x8, 5, 8, GLCD_Overwrite);

    GLCD_Clear();

    GLCD_GotoXY(0, 0);

    GLCD_PrintString("Bonjour !");

    GLCD_Render();

    while(1)
    {
        _delay_ms(1000);
    }
}

/* Méthode permettant d'allumer les LEDs en appuyant sur les boutons */
void turnLight(void) {
  CLKPR = 0b10000000; // modification du diviseur d'horloge (CLKPCE=1)
  CLKPR = 0;          // modification du diviseur d'horloge (CLKPCE=1)

  // LED4
  DDRD |= (1 << LED4);
  DDRB &= ~(1 << BOUTON4);
  PORTB |= (1 << BOUTON4);

  // LED5
  DDRD |= (1 << LED5);
  DDRB &= ~(1 << BOUTON5);
  PORTB |= (1 << BOUTON5);

  // LED6
  DDRD |= (1 << LED6);
  DDRB &= ~(1 << BOUTON6);
  PORTB |= (1 << BOUTON6);

  while (1)
  {
    #if 0
        // Bouton 4 → LED4
        if(PINB & (1 << BOUTON4))  
          PORTD &= ~(1 << LED4); 
        else                      
          PORTD |= (1 << LED4);

        // Bouton 5 → LED5
        if(PINB & (1 << BOUTON5))  
          PORTD &= ~(1 << LED5); 
        else                      
          PORTD |= (1 << LED5);

        // Bouton 6 → LED6
        if(PINB & (1 << BOUTON6))  
          PORTD &= ~(1 << LED6); 
        else                      
          PORTD |= (1 << LED6);

    #endif
    PORTD |= (1 << LED6);

    _delay_ms(1000);
    PORTD &= ~(1 << LED6);
    _delay_ms(1000);
  }
}