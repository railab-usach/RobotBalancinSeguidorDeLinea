/*
* El siguiente código es para aplicar una aceleración segura para
  que los motores no pasen de 0% a 100% bruscamente y se corra el
  peligro de quemar el bobinado.
* Valores recomendados en función de la inercia:
  - Motores pequeños: de 0% a 100% en 25ms --> máx 10,2 PWM/ms
  - Motores medianos: de 0% a 100% en 100ms --> máx 2.55 PWM/ms
  - Motores grandes: de 0% a 100% en 250ms --> máx 1 PWM/ms

* Se deben ingresar valores por el monitor serial y visualizar el serial plotter.
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
  //====================  MONITOR SERIAL  ====================
  Serial.begin(115200);
}


//######################################################################
//#########################  BUCLE PRINCIPAL  ##########################
//######################################################################

void loop() {
  static int pwmObjetivo = 0;
  //====================  MONITOR SERIAL  ====================
  if (Serial.available() > 0) {     // Lee el valor numerico enviado
    pwmObjetivo = Serial.parseInt();  //lo guarda en la variable
  }

  //====================  MOTORES  ====================
  
  //Paso MUY lento para que se visualice bien

  int intervalo = 50; //ms
  static unsigned long tiempoAnterior = 0;
  static int pwmActual = 0;
  if(millis() - tiempoAnterior >= intervalo){

    int diferencia = pwmObjetivo - pwmActual;
    int paso = 1;
    if(diferencia > paso){
      pwmActual += paso;
    }
    else if(diferencia < -paso){
      pwmActual -= paso;
    }
    else{
      pwmActual = pwmObjetivo;
    }
    //Limitar por si se ingresa un PWM incorrecto.
    pwmActual = constrain(pwmActual, -255, 255);
    motorA.setSpeed(pwmActual);
    motorB.setSpeed(pwmActual);
  }
  Serial.print(-255);
  Serial.print(",");
  Serial.print(255);
  Serial.print(",");
  Serial.println(pwmActual);
}
