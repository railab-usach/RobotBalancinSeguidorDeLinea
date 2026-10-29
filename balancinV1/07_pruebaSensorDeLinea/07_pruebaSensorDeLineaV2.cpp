/*
*  FUNCION: El siguiente código es para verificar las conexiones y la
  normal lectura del sensor.
*  NOTA:  El sensor de línea es analógico. Tras unas pruebas, la
  zona blanca marca alrededor de 2500 y en la zona negra 4000.
*/


//######################################################################
//##############################  PINES  ###############################
//######################################################################
int pinesInfrarrojo[8] = {36, 39, 34, 35, 32, 33, 25, 26};

//######################################################################
//####################  DECLARACION DE FUNCIONES  ######################
//######################################################################
int convertirAColor(int pinAnalogico);

//######################################################################
//####################  CONFIGURACIONES INICIALES  #####################
//######################################################################
void setup() {
  Serial.begin(115200);
}

//######################################################################
//#########################  BUCLE PRINCIPAL  ##########################
//######################################################################
void loop() {
  for(int i=0; i<8; i++){
    //Leer cada uno de los 8 sensores e imprimir su lectura
    Serial.print( convertirAColor( pinesInfrarrojo[i]));
  }
  Serial.println("");  //Salto de linea
  delay(200);
}

//######################################################################
//#####################  DEFINICION DE FUNCIONES  ######################
//######################################################################
//FUNCIÓN: Convertir las lecturas analógicas del sensor a digitales (para mejor comprensión)
//ENTRADAS: Pin de datos del sensor
//SALIDAS: Un 0 si el fondo es blanco y un 1 si el fondo es negro.
int convertirAColor(int pinAnalogico){
  if(analogRead(pinAnalogico) > 3500){
    return 1; //Color negro
  }
  else{
    return 0; //Color blanco
  }
}

