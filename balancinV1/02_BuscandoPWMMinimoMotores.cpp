/*
* El siguiente código es para encontrar el PWM mínimo con el cual los
  motores RECIEN empiezan a girar mínimamente (en la aire).
* Iremos ingresando diferentes PWM mediante el monitor Serial para
* aligerar la búsqueda.
* OBSERVACIÓN En el monitor serial, al lado de la velocidad,  hay que 
  configurar la opción "Sin ajuste de línea" para que no se guarde basura
  en la variable pwmMinimo.

* ============== RESULTADO ====================
* Con un pwm de 8 recien empieza a moverse el motor B.
* El motor A con un PWM de 10.
*/

//######################################################################
//##########################  LIBRERIAS  ###############################
//######################################################################

//====================  MOTORES  ====================
#include <CytronMotorDriver.h>


//######################################################################
//##############################  PINES  ###############################
//######################################################################

//====================  MOTORES  ====================
//Motor A
int M1A = 23;
int M1B = 22;

//Motor B
int M2A = 19;
int M2B = 21;

//====================  MONITOR SERIAL  ====================
int pwmMinimo = 0;


//######################################################################
//############################  OBJETOS  ###############################
//######################################################################

//====================  MOTORES  ====================
CytronMD motorA(PWM_PWM, M1A, M1B);
CytronMD motorB(PWM_PWM, M2A, M2B);


//######################################################################
//####################  CONFIGURACIONES INICIALES  #####################
//######################################################################

void setup() {
  //====================  MONITOR SERIAL  ====================
  Serial.begin(115200);
}


//######################################################################
//#########################  BUCLE PRINCIPAL  ##########################
//######################################################################

void loop() {
  //====================  MONITOR SERIAL  ====================
  if (Serial.available() > 0) {     // Lee el valor numerico enviado
    pwmMinimo = Serial.parseInt();  //lo guarda en la variable
  }

  //====================  MOTORES  ====================
  motorA.setSpeed(pwmMinimo);
  motorB.setSpeed(pwmMinimo);
}
