/*  
* El sensor de línea es analógico. Tras unas pruebas, la
  zona blanca marca alrededor de 2500 y en la zona negra 4000.
*/


// =========== PINES ================
int pinesInfrarrojo[8] = {36, 39, 34, 35, 32, 33, 25, 26};

//DECLARACION FUNCIONES
int convertirAColor(int pinAnalogico);

// ----- CONFIG INICIAL -----//
void setup() {
  Serial.begin(115200);
}

// ================= BUCLE PRINCIPAL
void loop() {
  for(int i=0; i<8; i++){
    Serial.print( convertirAColor( pinesInfrarrojo[i]));
  }
  Serial.println("");
  delay(200);
}


// DEFINICION FUNCIONES
int convertirAColor(int pinAnalogico){
  if(analogRead(pinAnalogico) > 3500){
    return 1; //Color negro
  }
  else{
    return 0; //Color blanco
  }
}
