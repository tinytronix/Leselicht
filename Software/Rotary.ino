//#define ENCODER_OPTIMIZE_INTERRUPTS
#define ENCODER_USE_INTERRUPTS
//#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>

#define IS_RELEASED 1
#define IS_PUSHED   0

//-------------------------------------------------------------------------------------
//
// globale Typdefinitionen
//
//-------------------------------------------------------------------------------------
typedef struct _tagPUSHBUTTON
{
  int           state;
  int           timer;
  int           debounceTimer;
  eBUTTONEVENT  event;
}PUSHBUTTON;


//-------------------------------------------------------------------------------------
//
// globale Objekte
//
//-------------------------------------------------------------------------------------
Encoder     encoder(PORT_ENCODER_DIRA, PORT_ENCODER_DIRB);
PUSHBUTTON  button;


//-------------------------------------------------------------------------------------
//
// Rotary_Init
//
//-------------------------------------------------------------------------------------
void Rotary_Init(void)
{
  pinMode(PORT_PUSHBUTTON, INPUT);
  
  button.state = BUTTON_IDLE;
  button.timer = 0;
  button.debounceTimer = 0;
}


//-------------------------------------------------------------------------------------
//
// Rotary_IsButtonPressed
//
//-------------------------------------------------------------------------------------
bool Rotary_IsButtonPressed(void)
{
  if ( 0 == digitalRead(PORT_PUSHBUTTON) )
    return true;
  else
    return false;
}


//-------------------------------------------------------------------------------------
//
// Rotary_Pushbutton
//
//-------------------------------------------------------------------------------------
eBUTTONEVENT Rotary_Pushbutton(void)
{
  eBUTTONEVENT ev = BUTTON_NOEVENT;
  
   int btn = digitalRead(PORT_PUSHBUTTON);

  if ( (button.timer > 0) && (button.timer < LONGPRESS_TIME) )
  {
    button.timer++;
  }
  
  switch (button.state)
  {
    case BUTTON_IDLE:
      if ( btn == IS_PUSHED )
      {
        button.debounceTimer = BUTTON_DEBOUNCE_TIME;
        button.state = BUTTON_DOWN;
        button.timer = 1;
      }
      break;

    case BUTTON_DOWN:
      if ( (button.debounceTimer > 0) && (btn == IS_RELEASED) )
      {
        button.debounceTimer = TIME_MILLISECONDS(200);
        button.state = BUTTON_WAITRELEASE;
      }
      else if ( button.debounceTimer > 0 )
      {
        button.debounceTimer--;
      }
      else if ( btn == IS_RELEASED ) 
      {
        if ( button.timer < LONGPRESS_TIME )
        {
          button.debounceTimer = BUTTON_DEBOUNCE_TIME;
          button.state = BUTTON_WAITDOUBLECLICK;
          button.timer = 1;
        }
      }
      else if ( button.timer >= LONGPRESS_TIME )
      {
         ev = BUTTON_LONGPRESS;
         button.state = BUTTON_WAITRELEASE;
      }
      break;

    case BUTTON_WAITDOUBLECLICK:
      if ( (button.debounceTimer > 0) && (btn == IS_PUSHED) )
      {
        button.debounceTimer = TIME_MILLISECONDS(200);
        button.state = BUTTON_WAITRELEASE;
      }
      else if (button.debounceTimer > 0 ) 
      {
        button.debounceTimer--;
      }
      else if ( btn == IS_PUSHED )
      {
        ev = BUTTON_DOUBLECLICK;
        button.state = BUTTON_WAITRELEASE;
        button.timer = 0;
      }
      else if ( button.timer > DOUBLECLICK_TIME ) 
      {
         button.timer = 0;
         ev = BUTTON_SHORTPRESS;
         button.state = BUTTON_IDLE;
      }
      break;

    case BUTTON_WAITRELEASE:
      if ( button.debounceTimer > 0 )
      {
        button.debounceTimer--;
      }
      else if ( btn == IS_RELEASED )
      {
        button.state = BUTTON_IDLE;
      }
      break;
  }
    
  return ev;
}


//-------------------------------------------------------------------------------------
//
// Rotary_ReadEncoder
//
//-------------------------------------------------------------------------------------
int Rotary_ReadEncoder(void)
{
  int rv = encoder.read();
  encoder.write(0);
  return rv;
}
