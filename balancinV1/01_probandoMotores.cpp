/*
* VERIFICACIÓN DE MOTORES:
* El siguiente código es para verificar las conexiones de los motores
  y el driver. Además de verificar el sentido de giro correcto.
* Se ocupan las instrucciones básicas para verificar movimiento.
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
}


//######################################################################
//#########################  BUCLE PRINCIPAL  ##########################
//######################################################################

void loop() {
  //====================  MOTORES  ====================
  //Avanzar a máxima velocidad Motor 1
  motorA.setSpeed(255);
  //Avanzar a máxima velocidad Motor 2
  motorB.setSpeed(255);
  delay(3000);

  //Detenerse
  motorA.setSpeed(0);
  motorB.setSpeed(0);
  delay(3000);

  //Retroceder a máxima velocidad Motor 1
  motorA.setSpeed(-255);
  //Retroceder a máxima velocidad Motor 2
  motorB.setSpeed(-255);

  delay(3000);

  //Acelerar
  for(int i=0; i<255; i++){ //Se demora 5s aprox en pasar de 0 a 255
    motorA.setSpeed(i); //Velocidad motor A
    motorB.setSpeed(i); //Velocidad motor B
    delay(20);
  }

  //Desacelerar
  for(int i=255; i>0; i--){ //Se demora 5s aprox en pasar de 255 a 0
    motorA.setSpeed(i); //Velocidad motor A
    motorB.setSpeed(i); //Velocidad motor B
    delay(20);
  }
}
