/*
* El siguiente código es para verificar el correcto funcionamiento y
  medición del ángulo de inclinación del MPU6050.
* A través del monitor Serial debería imprimirse el ángulo de inclinación
  respecto al eje X.
* OBSERVACIÓN: Recordar calibrar los offsets

* ============== RESULTADO ====================
* El MPU mide bien, pero el centro de gravedad está en: -1.06°
*/

//######################################################################
//##########################  LIBRERIAS  ###############################
//######################################################################

//====================  I2C  ====================
#include "Wire.h"                           //Libreria para activar comunicación I2C.
#include "I2Cdev.h"                         //Libreria auxiliar para más comandos con I2C (necesaria para mpu6050.h).

//====================  MPU6050  ====================
#include "MPU6050.h"


//######################################################################
//##############################  PINES  ###############################
//######################################################################

//====================  I2C  ====================
int pinSDA = 16;
int pinSCL = 17;


//######################################################################
//############################  OBJETOS  ###############################
//######################################################################

//====================  MPU6050  ====================
MPU6050 mpu(0x68);      //Creamos una categoría tipo MPU6050 y agregamos un dispositivo llamado mpu.
                        //El parámetro es su dirección I2C.


//######################################################################
//######################  VARIABLES GLOBALES  ##########################
//######################################################################

//====================  MPU6050  ====================
// Variables para almacenar datos crudos (16 bits)
int16_t ax, ay, az;
int16_t gx, gy, gz;


//######################################################################
//####################  CONFIGURACIONES INICIALES  #####################
//######################################################################
void setup() {
  /*----- CONFIG I2C -----*/
  Wire.begin(pinSDA, pinSCL);               //Inicia el I2C.
  Wire.setClock(400000);      //Aumenta la velocidad de la comunicación I2C porque el DMP envía demasiados datos.
  Serial.begin(115200);       //Inicia la comunicación Serial.


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
}


//######################################################################
//#########################  BUCLE PRINCIPAL  ##########################
//######################################################################
void loop() {

  // Bucle de lectura a 100Hz (cada 10ms)
  static unsigned long tiempoAnterior = 0;
  if (millis() - tiempoAnterior >= 10) {
    tiempoAnterior = millis();

    // Obtención directa de los 6 datos crudos
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // Imprimir datos en bruto
    Serial.print("AccelRAW Z:"); Serial.print(az);
    Serial.print(" | GyroRAW X:"); Serial.println(gx);
  }
}


