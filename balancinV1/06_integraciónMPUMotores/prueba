/*
* FUNCIÓN: El siguiente código es para calibrar con cuenta fuerza se corrije la posición
  del balancín al leer la línea.
*  PROCEDIMIENTO: Se tiene que generar un setpoint un poquito más alto que el setpoint de equilibrio,
  esto con la finalidad de que el balancín avance. Posterior a esto, se tiene que asignar valores al
  "factor de linea". Este número indica con cuanta fuerza se gira a la derecha o izquierda para seguir
  la linea.
  Los valores se van asignando por bluetooth con la app Serial bluetooth terminal. Dentro del loop está
  el bloque perteneciente al bluetooth con 2 comandos (setpoint y kl).

* ================== RESULTADOS ======================
*  Con un setpoint de entre 3.5° y 4° el balancín avanza de forma relativamente controlada.
*  Con un factor de linea igual a 10 el balancín tiene la suficiente fuerza para seguir la línea,
  pero sin girar en exceso para salirse del camino.
*/


//######################################################################
//############################  LIBRERIAS  #############################
//######################################################################
//====================  I2C  ====================
#include "Wire.h"                           //Libreria para activar comunicación I2C.
#include "I2Cdev.h"                         //Libreria auxiliar para más comandos con I2C.

//====================  MPU6050  ====================
#include "MPU6050.h"

//====================  MOTOR  ====================
#include <CytronMotorDriver.h>

//====================  BLUETOOTH  ====================
#include "BluetoothSerial.h"

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

//====================  I2C  ====================
int pinSDA = 16;
int pinSCL = 17;

//====================  SENSOR INFRARROJO  ====================
int pinesInfrarrojo[8] = {36, 39, 34, 35, 32, 33, 25, 26};

//######################################################################
//######################  VARIABLES GLOBALES  ##########################
//######################################################################
//====================  PID MOTOR - MPU6050  ====================
double setpointIMU = 2.6;    //Desfase IMU

//====================  MOTOR - LINEA  ====================
float factorLinea = 0;

//Peso de la línea a seguir:
int peso[8] = {-7,-5,-3,-1,1,3,5,7};

//######################################################################
//############################  OBJETOS  ###############################
//######################################################################
//====================  MPU6050  ====================
MPU6050 mpu(0x68);      //Creamos una categoría tipo MPU6050 y agregamos un dispositivo llamado mpu.
                        //El parámetro es su dirección I2C.

//====================  MOTORES  ====================
CytronMD motorA(PWM_PWM, M1A, M1B);
CytronMD motorB(PWM_PWM, M2A, M2B);

//====================  BLUETOOTH  ====================
BluetoothSerial SerialBT;

//######################################################################
//####################  DECLARACION FUNCIONES  #########################
//######################################################################
//====================  MOTORES  ====================
int rampaPWM(int &pwmActual, int &pwmObjetivo, int paso);

//====================  SENSOR INFRARROJO  ====================
int convertirAColor(int pinAnalogico);

//######################################################################
//####################  CONFIGURACIONES INICIALES  #####################
//######################################################################
void setup() {
  //====================  I2C  ====================
  //Config I2C
  Wire.begin(pinSDA, pinSCL);               //Inicia el I2C.
  Wire.setClock(400000);

  //====================  BLUETOOTH  ====================
  SerialBT.begin("MiBalancin"); //Nombre de nuestro dispositivo bluetooth

  //====================  MPU6050  ====================
  /*----- INICIAR MPU -----*/
  mpu.initialize();       //Inicia el mpu (pero el DMP aún no).

  /*----- CALIBRACIÓN MPU -----*/
  mpu.setDLPFMode(3);  //Filtro pasabajos: Corte = 42hz
  mpu.setFullScaleGyroRange(1); //Resolución giroscopio: 500°/s
  mpu.setFullScaleAccelRange(2); //Resolución acelerómetro: 8g

  // Carga de tus offsets
  mpu.setXGyroOffset(137);
  mpu.setYGyroOffset(-72);
  mpu.setZGyroOffset(19);

  mpu.setXAccelOffset(2193);
  mpu.setYAccelOffset(215);
  mpu.setZAccelOffset(1151);

  //======== CÓDIGO DE ESTABILIZACIÓN ===================
  delay(2000);

  //Descartar las primeras lecturas basura
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  for(int i = 0; i < 20; i++){
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    delay(5);
  }
}

//######################################################################
//#########################  BUCLE PRINCIPAL  ##########################
//######################################################################
void loop() {
   //====================  SISTEMA DE CONTROL  ====================
  // Bucle de lectura a 200Hz (cada 5ms)
  static unsigned long tiempoAnteriorPID = 0;
  int intervaloPIDIMU = 5000;  //5microseg: T=1/0.005s -> f = 200hz
  if (micros() - tiempoAnteriorPID >= intervaloPIDIMU) {
    tiempoAnteriorPID = micros();

    //====================  SENSADO  ====================
    //Obtención directa de los 6 datos crudos
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    //Conversión de datos digitales a unidades fisicas (grados/segundo)
    float giroX = gx/65.5; //grados. El 65.5 es el valor correspondiente a la resolución de 500°/s

    //Mi balancín gira en el eje x. Por lo tanto, se mueve en el eje Y, Z (de forma lineal).
    float anguloAcel = atan2(ay, sqrt((long)az*az + (long)ax*ax))*180/PI;  //Calculo en 3D para compensar cualquier desviación
    //No transformamos los datos digitales de la aceleración a unidades física
    //  porque con la operación atan2 se cancelan las unidades.

    float dt = 0.005;
    static float rollFiltrado = 0.0;
    rollFiltrado = 0.98 * (rollFiltrado + giroX * dt) + 0.02 * anguloAcel;

    //====================  LÓGICA DE CONTROL 1: IMU-MOTOR ====================
    double inputIMU = rollFiltrado;    //Medición del angulo roll del MPU6050 para el PID.

    float error = inputIMU - setpointIMU;

    //Parte proporcional:
    double kpIMU = 30;
    float proporcional = kpIMU * error;

    //Parte integral:
    static float errorSum = 0;

    //Acumulación del error por cada dt (5ms = 5000 microsegundos)
    double kiIMU = 500;
    errorSum += error * dt;
    if(kiIMU == 0){
      errorSum = 0; //Para evitar que el error se acumule antes de aplicar la parte integral
    }
    // Anti-Windup: limita la acumulación integral para evitar saturación
    errorSum = constrain(errorSum, -150.0, 150.0); 

    float integral = kiIMU * errorSum;

    //Parte derivativa:
    double kdIMU = 0.5;
    float derivativo = kdIMU * giroX;

     // ---- CORTE DE SEGURIDAD POR CAÍDA ----
    if (abs(rollFiltrado) > 40.0) { 
      // Si el robot cae más de 40 grados, apaga todo
      double outputIMU = 0;          // Limpia la salida residual
      errorSum = 0; //Eliminar error
    } else {
      // Operación normal
        
      double outputIMU = proporcional + integral + derivativo;
      // 2. Acotación de la salida
      outputIMU = constrain(outputIMU, -255.0, 255.0);

      //====================  LÓGICA DE CONTROL 2: LINEA-MOTOR ====================
      float errorLinea = 0.0;
      int cantLeidos, posicion = 0;
      for(int i=0; i<8; i++){
        posicion += convertirAColor(pinesInfrarrojo[i])* peso[i];
        cantLeidos += convertirAColor(pinesInfrarrojo[i]);
      }
      if (cantLeidos != 0){
        //El error es un número que indica si la línea está hacía la derecha o izquierda
        errorLinea = posicion/cantLeidos;
      }else{
        //Detener avance del balancin
      }

      //====================  ACTUACION  ====================
      //Compensación de la zona muerta con el PWM mínimo útil.
      int minPWMA = 10;
      int minPWMB = 8;
      float pwmUtilA = 0;
      float pwmUtilB = 0;
      float factorMotorB = 0.78; //Motor que gira más rápido

      if (outputIMU > 0) {
        //Reescalado de la zona muerta
        pwmUtilA = map(outputIMU, 1,255, minPWMA, 255);
        pwmUtilB = map(outputIMU, 1,255, minPWMB, 255) * factorMotorB;
        //Corrección de la línea
        pwmUtilA += errorLinea * factorLinea;
        pwmUtilB -= errorLinea * factorLinea;
      } else if (outputIMU < 0) {
        //Reescalado de la zona muerta
        pwmUtilA = map(outputIMU, -1,-255, -minPWMA, -255);
        pwmUtilB = map(outputIMU, -1,-255, -minPWMB, -255) * factorMotorB;
        //Corrección de la línea
        pwmUtilA -= errorLinea * factorLinea;
        pwmUtilB += errorLinea * factorLinea;
      }else{
        pwmUtilA = 0;
        pwmUtilB = 0;
      }
      
      //Limitar los valores de la variable
      pwmUtilA = constrain(pwmUtilA, -255, 255);
      pwmUtilB = constrain(pwmUtilB, -255, 255);
      
      //Rampa de aceleración
      int paso = 25;  //25PWM/5ms = 5 PWM/ms
      static int pwmActualA = 0;
      static int pwmActualB = 0;
      int pwmObjetivoA = pwmUtilA;
      int pwmObjetivoB = pwmUtilB;
      pwmActualA = rampaPWM(pwmActualA, pwmObjetivoA, paso);
      pwmActualB = rampaPWM(pwmActualB, pwmObjetivoB, paso);

      motorA.setSpeed(pwmActualA);
      motorB.setSpeed(pwmActualB);
    
    }
  }

  //====================  BLUETOOTH IMU  ====================
  static String comandoBT = "";

  while (SerialBT.available()) {
    char c = SerialBT.read();

    if (c == '\n' || c == '\r') {
      if (comandoBT.length() > 0) {
        comandoBT.trim();

        if (comandoBT.startsWith("kl:")) {
          factorLinea = comandoBT.substring(3).toFloat();
        }
        else if(comandoBT.startsWith("setpoint:")) {
          setpointIMU = comandoBT.substring(9).toFloat();
        }
        comandoBT = "";
      }
    } else {
      comandoBT += c;
      if (comandoBT.length() > 64) {
        comandoBT = "";
      }
    }
  }
}

//######################################################################
//#####################  DEFINICION FUNCIONES  #########################
//######################################################################
//FUNCION: Acelerar de manera controlada el pwm para motores
//ENTRADA 1: pwm actual del motor
//ENTRADA 2: pwm al que se quiere llegar
//ENTRADA 3: aumento del pwm en cada ciclo
//SALIDA: nuevo pwm actual
int rampaPWM(int &pwmActual, int &pwmObjetivo, int paso){
  int diferencia = pwmObjetivo - pwmActual;

  bool cambioSentido = (pwmObjetivo > 0 && pwmActual < 0) || (pwmObjetivo < 0 && pwmActual > 0);
  if (cambioSentido){
    paso = paso/3;    //Relentizar paso
  }

  //ASIGNACIÓN DE PASO A LA SALIDA
  if(diferencia > paso) pwmActual += paso;        //Si aún hay diferencia positiva, aumentar pwm
  else if(diferencia < -paso) pwmActual -= paso;  //Si hay diferencia negativa, disminuir pwm
  else pwmActual = pwmObjetivo;                   //Si la diferencia es muy pequeña, igualar al pwmObjetivo

  //RECTIFICAR LÍMITES
  pwmActual = constrain(pwmActual, -255, 255);

  return pwmActual;
}

int convertirAColor(int pinAnalogico){
  if(analogRead(pinAnalogico) > 3500){
    return 1; //Color negro
  }
  else{
    return 0; //Color blanco
  }
}
