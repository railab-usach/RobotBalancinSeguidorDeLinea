/*
* FUNCIÓN: El siguiente código es para aplicar una aceleración segura para
  que los motores no pasen de 0% a 100% bruscamente y mucho menos
  de -100% a 100% (cambio de dirección) y se corra el peligro de
  quemar el bobinado por la "corriente de irrupción transitoria".
* TABLA DE VALORES: Los valores recomendados son muy generales y dependen
   de la inercia del motor y considerando un PWM max de 255:
  - Motores pequeños:
    - Mismo Sentido (Aceleración): 10 a 25 PWM/ms.
    - Cambio de Sentido (Inversión): 2 a 5 PWM/ms
  - Motores medianos:
    - Mismo Sentido (Aceleración): 3 a 8 PWM/ms.
    - Cambio de Sentido (Inversión): 1 a 3 PWM/ms
  - Motores grandes:
    - Mismo Sentido (Aceleración): 1 a 3 PWM/ms.
    - Cambio de Sentido (Inversión): 0.3 a 1 PWM/ms

* PROCEDIMIENTO: Se deben ir ingresando valores por el Serial Plotter y se
  visualizará la gráfica de cambio paulatino de velocidad.
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
//####################  DECLARACION FUNCIONES  #########################
//######################################################################
int rampaPWM(int &pwmActual, int &pwmObjetivo, int paso);

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
    int paso = 100;
    pwmActual = rampaPWM(pwmActual, pwmObjetivo, paso);
    motorA.setSpeed(pwmActual);
    motorB.setSpeed(pwmActual);
  }

  //====================  MONITOR SERIAL  ====================
  Serial.print(-255);
  Serial.print(",");
  Serial.print(255);
  Serial.print(",");
  Serial.println(pwmActual);
}

//######################################################################
//#####################  DEFINICION FUNCIONES  #########################
//######################################################################
//FUNCION: Acelerar de manera controlada el pwm para motores
//ENTRADA 1: pwm actual del motor
//ENTRADA 2: pwm al que se quiere llegar
//ENTRADA 3: aumento del pwm en cada ciclo (paso)
//SALIDA: nuevo pwm actual
int rampaPWM(int &pwmActual, int &pwmObjetivo, int paso){

  //Actualizar valor del paso si es que se detecta un cambio de sentido
  if (pwmObjetivo > 0 && pwmActual < 0) || (pwmObjetivo < 0 && pwmActual > 0){
    paso = paso/3;    //Relentizar paso
  }

  //ASIGNACIÓN DE PASO A LA SALIDA
  int diferencia = pwmObjetivo - pwmActual;
  if(diferencia > paso) pwmActual += paso;        //Si aún hay diferencia positiva, aumentar pwm
  else if(diferencia < -paso) pwmActual -= paso;  //Si hay diferencia negativa, disminuir pwm
  else pwmActual = pwmObjetivo;                   //Si la diferencia es muy pequeña, igualar al pwmObjetivo

  //RECTIFICAR LÍMITES
  pwmActual = constrain(pwmActual, -255, 255);

  return pwmActual;
}

