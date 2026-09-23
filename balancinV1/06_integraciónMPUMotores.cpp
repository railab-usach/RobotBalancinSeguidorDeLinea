/*
* El siguiente código tiene como finalidad lograr que el balancin
  mantenga el equilibrio. No se busca necesariamente que sea extremadamente
  estable, pues eso se hará más adelante al integrar encoders.
* Los valores se van asignando por bluetooth con la app Serial bluetooth terminal

* ================== RESULTADOS ======================
* kp: 30, ki= 500, kd: 0.1

* ================== METODOLOGIA ======================
* 1. Iniciar con kp,ki,kd en 0
* 2. Subir kp hasta llega a un valor donde el balancin trate de mantenerse en equilibrio 
    oscile 1 o 2 veces y se caiga. (el balanceo debe ser muy leve. Si se inclina más de 5° se caerá)
* 3. Subir kd de a poco hasta que el balancín deje de oscilar
* 4. El balancín se seguirá cayendo. Aumentar ki con el valor necesario hasta
    que deje de caerse y soporte empujones.
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
//#######################  VARIABLES GLOBALES  #########################
//######################################################################

//====================  MPU6050  ====================
// Variables para almacenar datos crudos (16 bits)
int16_t ax, ay, az;
int16_t gx, gy, gz;
float rollFiltrado = 0.0;

//====================  PID MOTOR IMU  ====================
double setpointIMU, inputIMU, outputIMU; //Variables dinámicas IMU.
double kpIMU = 0, kiIMU = 0, kdIMU = 0;  //Parámetros IMU.

int intervaloPIDIMU = 5000;  //5us: T=1/0.005s -> f = 200hz

int minPWMA = 8;
int minPWMB = 6;

int pwmActualA = 0;
int pwmActualB = 0;


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
//####################  CONFIGURACIONES INICIALES  #####################
//######################################################################
void setup() {
  //====================  I2C  ====================
  //Config I2C
  Wire.begin(pinSDA, pinSCL);               //Inicia el I2C.
  Wire.setClock(400000);

  Serial.begin(115200);
  //====================  BLUETOOTH  ====================
  SerialBT.begin("Balancin_ESP32"); //Nombre de nuestro dispositivo I2C

  //====================  MPU6050  ====================
  /*----- INICIAR MPU -----*/
  mpu.initialize();       //Inicia el mpu (pero el DMP aún no).


  /*----- CALIBRACIÓN MPU -----*/
  mpu.setDLPFMode(MPU6050_DLPF_BW_42);  //Filtro pasabajos
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250); //Resolución giroscopio
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2); //Resolución acelerómetro

  // Carga de tus offsets
  mpu.setXGyroOffset(137);
  mpu.setYGyroOffset(-72);
  mpu.setZGyroOffset(19);

  mpu.setXAccelOffset(2193);
  mpu.setYAccelOffset(215);
  mpu.setZAccelOffset(1151);

  //====================  PID IMU  ====================
  setpointIMU = 0.6;                       //Desfase IMU

  //======== CÓDIGO DE ESTABILIZACIÓN ===================
  delay(2000);

  //Descartar las primeras lecturas basura
  for(int i = 0; i < 20; i++){
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    delay(5);
  }

  //Pre-cargar el filtro complementario con el estado inicial real
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  rollFiltrado = atan2(ay, az) * 180 / PI;
}


//######################################################################
//#########################  BUCLE PRINCIPAL  ##########################
//######################################################################
void loop() {

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
        else if(comandoBT.startsWith("pwma:")) {
          minPWMA = comandoBT.substring(5).toFloat();
        }
        else if(comandoBT.startsWith("pwmb:")) {
          minPWMB = comandoBT.substring(5).toFloat();
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


  //====================  MPU6050  ====================
  // Bucle de lectura a 200Hz (cada 5ms)
  static unsigned long tiempoAnteriorPID = 0;
  static unsigned long tiempoAnteriorDt = 0;
  if (micros() - tiempoAnteriorPID >= 5000) {
    tiempoAnteriorPID = micros();

    // Obtención directa de los 6 datos crudos
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    //Conversión de datos digitales a unidades fisicas
    float giroX = gx/131.0; //grados. El 131 es el valor correspondiente a la resolución

    //Mi balancín gira en el eje x. Por lo tanto, se mueve en el eje Y, Z (de forma lineal).
    float anguloAcel = atan2(ay, az)*180/PI;
    //No calculamos la aceleración en m/s^2 porque con la operación atan2 se cancelan las unidades.

    rollFiltrado = 0.98 * (rollFiltrado + giroX * dt) + 0.02 * anguloAcel;

    //====================  MOTOR - PID - MPU6050  ====================
      
    inputIMU = rollFiltrado;    //Medición del angulo roll del MPU6050 para el PID.

    //Parte proporcional:
    float error = inputIMU - setpointIMU;
    float proporcional = kpIMU * error;

    //Parte integral:
    static float errorSum = 0;
    //Acumulación explícita multiplicada por el dt real
    float dt = (micros() - tiempoAnteriorDt) / 1000000.0; //Transformación microsegundos a segundos
    errorSum += error * dt;
    if(kiUMU == 0){
      errorSum = 0; //Para evitar que el error se acumule antes de aplicar la parte integral
    }
    // Anti-Windup: limita la acumulación integral para evitar saturación
    errorSum = constrain(errorSum, -50.0, 50.0); 
    float integral = kiIMU * errorSum;

    //Parte derivativa:
    float derivativo = kdIMU * giroX;


    // ---- CORTE DE SEGURIDAD POR CAÍDA ----
    if (abs(rollFiltrado) > 40.0) { 
      // Si el robot cae más de 40 grados, apaga todo
      motorA.setSpeed(0);
      motorB.setSpeed(0);
      outputIMU = 0;          // Limpia la salida residual
      errorSum = 0; //Eliminar error
      pwmActualA = 0;
      pwmActualB = 0;
    } else {
      // Operación normal
        
      outputIMU = proporcional + integral + derivativo;
      // 2. Acotación de la salida
      outputIMU = constrain(outputIMU, -255.0, 255.0);


      //Compensación de la zona muerta con el PWM mínimo útil.
      float pwmUtilA = 0;
      float pwmUtilB = 0;

      if (outputIMU > 0) {

        pwmUtilA = map(outputIMU, 0,255, minPWMA, 255);
        pwmUtilB = map(outputIMU, 0,255, minPWMB, 255);
      } else if (outputIMU < 0) {
        pwmUtilA = map(outputIMU, 0,-255, -minPWMA, -255);
        pwmUtilB = map(outputIMU, 0,-255, -minPWMB, -255);
      }else{
        pwmUtilA = 0;
        pwmUtilB = 0;
      }
      
      //Limitar los valores de la variable
      pwmUtilA = constrain(pwmUtilA, -255, 255);
      pwmUtilB = constrain(pwmUtilB, -255, 255);

      //Rampa de aceleración
      int diferenciaA = pwmUtilA - pwmActualA;
      int diferenciaB = pwmUtilB - pwmActualB;
      int paso = 15;
      if(diferenciaA > paso){
        pwmActualA += paso;
      }
      else if(diferenciaA < -paso){
        pwmActualA -= paso;
      }
      else{
        pwmActualA = pwmUtilA;
      }

      if(diferenciaB > paso){
        pwmActualB += paso;
      }
      else if(diferenciaB < -paso){
        pwmActualB -= paso;
      }
      else{
        pwmActualB = pwmUtilB;
      }

      motorA.setSpeed(pwmActualA);
      motorB.setSpeed(pwmActualB);
    }
  }
}
