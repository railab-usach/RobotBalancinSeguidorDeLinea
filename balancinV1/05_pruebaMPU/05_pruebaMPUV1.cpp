/*
* FUNCIÓN: El siguiente código es para verificar el correcto funcionamiento y
  medición del ángulo de inclinación del MPU6050.
  A través del monitor Serial debería imprimirse el ángulo de inclinación
  respecto al eje X.
* NOTA: Recordar calibrar los offsets
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
MPU6050 mpu(0x68);      //Creamos un objeto tipo MPU6050 llamado mpu

//######################################################################
//######################  VARIABLES GLOBALES  ##########################
//######################################################################
//====================  MPU6050  ====================
// Variables para almacenar datos crudos (16 bits)
int16_t ax, ay, az;
int16_t gx, gy, gz;
float rollFiltrado = 0.0;

//######################################################################
//####################  CONFIGURACIONES INICIALES  #####################
//######################################################################
void setup() {
  //====================  MPU6050  ====================
  /*----- CONFIG I2C -----*/
  Wire.begin(pinSDA, pinSCL);               //Inicia el I2C.
  Wire.setClock(400000);      //Aumenta la velocidad de la comunicación I2C.

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

  //====================  MONITOR SERIAL  ====================
  Serial.begin(115200);       //Inicia la comunicación Serial.
}

//######################################################################
//#########################  BUCLE PRINCIPAL  ##########################
//######################################################################
void loop() {
  //====================  MPU6050  ====================
  // Bucle de lectura a 200Hz (cada 5ms)
  static unsigned long tiempoAnterior = 0;
  if (micros() - tiempoAnterior >= 5000) {

    float dt = (micros() - tiempoAnterior) / 1000000; //Transformamos a segundos
    tiempoAnterior = micros();

    // Obtención directa de los 6 datos crudos
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    //Conversión de datos digitales a unidades fisicas
    float giroX = gx/131.0;
    float giroY = gy/131.0;
    float giroZ = gz/131.0; //El 131 es el valor correspondiente a la resolución

    //El balancín gira sobre el eje x. Por lo tanto, se generan componentes en los ejes Y, Z.
    float anguloAcel = atan2(ay, az)*180/PI;
    //No calculamos la aceleración en m/s^2 porque con la operación atan2 se cancelan las unidades.

    //FILTRO COMPLEMENTARIO (para tener una medición del ángulo más suave y estable)
    rollFiltrado = 0.98 * (rollFiltrado + giroX * dt) + 0.02 * anguloAcel;

    //====================  MONITOR SERIAL  ====================
    //Imprimir ángulo:
    Serial.print("Ángulo: ");
    Serial.println(rollFiltrado);
  }
}
