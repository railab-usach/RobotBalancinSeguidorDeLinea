/*
* El siguiente código es para aplicar una aceleración segura para
  que los motores no pasen de 0% a 100% bruscamente y mucho menos
  de -100% a 100% (cambio de dirección) y se corra el peligro de
  quemar el bobinado.
* Valores recomendados en función de la inercia y considerando el PWM max 255:
  - Motores pequeños:
    - Mismo Sentido (Aceleración): 10 a 25 PWM/ms.
    - Cambio de Sentido (Inversión): 2 a 5 PWM/ms
  - Motores medianos:
    - Mismo Sentido (Aceleración): 3 a 8 PWM/ms.
    - Cambio de Sentido (Inversión): 1 a 3 PWM/ms
  - Motores grandes:
    - Mismo Sentido (Aceleración): 1 a 3 PWM/ms.
    - Cambio de Sentido (Inversión): 0.3 a 1 PWM/ms

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
  static int pwmActual = 0;

  int intervalo = 50; //ms
  static unsigned long tiempoAnterior = 0;
  if(millis() - tiempoAnterior >= intervalo){

  int diferencia = pwmObjetivo - pwmActual;

  //CONTROL DEL PASO
  //Aceleración: 100PWM/50ms = 2 PWM/ms
  int paso = 100; // Permite cambios rápidos pero acota el pico extremo

  //Detecta si hay un cambio de sentido (inversión brusca de polaridad)
  bool cambioDeSentido = (pwmObjetivo > 0 && pwmActual < 0) || (pwmObjetivo < 0 && pwmActual > 0);
  if (cambioDeSentido) {
    //Inversión: 2PWM/50ms = 0.04 PWM/ms
    paso = 2; //Paso MUY lento para que se visualice bien
  }

  //ASIGNACIÓN DE PASO A LA SALIDA
  if(diferencia > paso){
      pwmActual += paso;
    }
    else if(diferencia < -paso){
      pwmActual -= paso;
    }
    else{
      pwmActual = pwmObjetivo;
    }
    pwmActual = constrain(pwmActual, -255, 255);

    //MOVER MOTOR
    motorA.setSpeed(pwmActual);
    motorB.setSpeed(pwmActual);
  }
  Serial.print(-255);
  Serial.print(",");
  Serial.print(255);
  Serial.print(",");
  Serial.println(pwmActual);
}
