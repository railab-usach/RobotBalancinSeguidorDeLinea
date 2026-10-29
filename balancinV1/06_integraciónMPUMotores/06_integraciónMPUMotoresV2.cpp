/*
* FUNCIÓN: El siguiente código tiene como finalidad lograr que el balancin
  mantenga el equilibrio. No se busca necesariamente que sea extremadamente
  estable, pues eso se hará más adelante al integrar encoders.
* PROCEDIMIENTO: Los valores del control PID se van asignando por bluetooth
  con la app Serial bluetooth terminal. Dentro del loop está el bloque perteneciente
  al bluetooth con 5 comandos (setpoint, kp, kd, ki y pwmbmax)
  
* ================== METODOLOGIA ======================
* 1. Iniciar con kp,ki,kd en 0
* 2. Subir kp de 1 a 1 o 5 en 5 hasta llega a un valor donde el balancin trate de mantenerse en equilibrio 
    oscile 1 o 2 veces y se caiga. (el balanceo debe ser muy leve. Si el balancín supera los 5°-10°
    de inclinación se caerá). Si se asigna un kp muy alto, el balancín oscilará bruzcamente.
* 3. Subir kd de a poco (de 0,1 en 0,1 o incluso 0.05 en 0.05) hasta que el balancín deje de oscilar. Si
  el balancín empieza a vibrar es porque kd es muy alto. El balancín se seguirá cayendo ante
  fuertes perturbaciones.
* 4. Aumentar ki de 10 en 10 hasta que el balancín deje de caerse y soporte empujones.

* ================== RESULTADOS PID ======================
* kp: 30, ki= 500, kd: 0.5
* NOTA: Los motores funcionaban de forma dispareja, es decir, uno giraba más rápido que el otro, generando que el
  balancín se equilibre, pero con el tiempo empiece a girar hacía un lado. Para solucionar
  esto, se agregó un factor de reducción al motor más rápido. Este factor es un número de 0 al 1 que se multiplica
  al pwm del motor. Un factor de 0.5 significa que el motor funcionará a su 50%.
  RESULTADO: Luego de diferentes pruebas de balanceo, el factor adecuado fue de 0.78.
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

//######################################################################
//######################  VARIABLES GLOBALES  ##########################
//######################################################################
//====================  PID IMU-MOTOR  ====================
double kpIMU, kiIMU, kdIMU = 0.0;
double setpointIMU = 2.6;    //Desfase IMU
float factorMotorB = 1; //Motor que gira más rápido

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

  //======== CÓDIGO DE ESTABILIZACIÓN MPU ===================
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
  //====================  SISTEMA DE CONTROL 1  ====================
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

    //====================  LÓGICA DE CONTROL  ====================
    double inputIMU = rollFiltrado;    //Medición del angulo roll del MPU6050 para el PID.

    float error = inputIMU - setpointIMU;

    //PARTE PROPORCIONAL:
    float proporcional = kpIMU * error;

    //PARTE INTEGRAL:
    static float errorSum = 0;

    //Acumulación del error por cada dt (5ms = 5000 microsegundos)
    errorSum += error * dt;
    if(kiIMU == 0){
      errorSum = 0; //Para evitar que el error se acumule en las pruebas iniciales
    }
    // Anti-Windup: limita la acumulación integral para evitar saturación
    errorSum = constrain(errorSum, -150.0, 150.0); 

    float integral = kiIMU * errorSum;

    //PARTE DERIVATIVA
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

      //====================  ACTUACION  ====================
      //Compensación de la zona muerta con el PWM mínimo útil.
      int minPWMA = 10;
      int minPWMB = 8;
      float pwmUtilA = 0;
      float pwmUtilB = 0;

      if (outputIMU > 0) {
        pwmUtilA = map(outputIMU, 1,255, minPWMA, 255);
        pwmUtilB = map(outputIMU, 1,255, minPWMB, 255) * factorMotorB;
      } else if (outputIMU < 0) {
        pwmUtilA = map(outputIMU, -1,-255, -minPWMA, -255);
        pwmUtilB = map(outputIMU, -1,-255, -minPWMB, -255) * factorMotorB;
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

        if (comandoBT.startsWith("kp:")) {
          kpIMU = comandoBT.substring(3).toFloat();
        }
        else if(comandoBT.startsWith("ki:")) {
          kiIMU = comandoBT.substring(3).toFloat();
        }
        else if(comandoBT.startsWith("kd:")) {
          kdIMU = comandoBT.substring(3).toFloat();
        }
        else if(comandoBT.startsWith("setpoint:")) {
          setpointIMU = comandoBT.substring(9).toFloat();
        }
        else if(comandoBT.startsWith("pwmbmax:")) {
          factorMotorB = comandoBT.substring(8).toFloat();
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

  //Actualizar valor del paso si es que se detecta un cambio de sentido
  if (pwmObjetivo > 0 && pwmActual < 0) || (pwmObjetivo < 0 && pwmActual > 0){
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
