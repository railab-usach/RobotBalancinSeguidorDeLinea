/*
* El siguiente código lee si es que hay conexiones I2C disponibles. Útil para
  verificar si hay errores o no en la comunicación o conexión física.
* Si hay dispositivos I2C conectados correctamente, se arrojará sus direcciones
  por el monitor serial, sino, dirá que no hay conexiones disponibles.

* IMPORTANTE: Solo debes leer y configurar el apartado "PINES I2C" y
  "UNICA CONFIGURACIÓN A REALIZAR". Todo el resto del código puede ser
  ignorado y compilar directamente. 
*/

//######################################################################
//###########################  PINES I2C  ##############################
//######################################################################
//  Si ocupas una ESP32 y quieres definir como pines I2C otros que no son
//  los que vienen configurados por defecto las siguientes dos variables
//  deben estar descomentadas y debes escribir el número de pin.
#include <Wire.h>

int pinSDA = 16;
int pinSCL = 17;

void setup() {
  //######################################################################
  //################  UNICA CONFIGURACION A REALIZAR  ####################
  //######################################################################

  //  Si ocupas una ESP32 y descomentaste las variables pinSDA y pinSCL debes
  //  comentar el Wire.begin(); y descomentar Wire.begin(pinSDA, pinSCL);
  
  //Wire.begin(); //Comentar
  Wire.begin(pinSDA, pinSCL); //Descomentar

  //  Si ocupas un arduino deja este valor por defecto. Si ocupas una ESP32
  //  es recomendable cambiarlo a 115200
  Serial.begin(115200);

  //====================  NO TOCAR NADA MÁS  ====================




  
  while (!Serial); 
  Serial.println("\nI2C Scanner");
}

void loop() {
  byte error, address;
  int nDevices;

  Serial.println("Buscando dispositivos...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    // El scanner llama a cada dirección posible
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("!Dispositivo I2C encontrado en la direccion 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Error desconocido en la direccion 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No se encontraron dispositivos I2C\n");
  else
    Serial.println("busqueda terminada\n");

  delay(5000); // Espera 5 segundos y busca de nuevo
}
