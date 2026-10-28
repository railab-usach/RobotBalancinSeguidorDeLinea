/*  
* El sensor de línea es analógico. Tras unas pruebas, la
  zona blanca marca alrededor de 2500 y en la zona negra 4000.
*/


// =========== PINES ================
int pinesInfrarrojo[8] = {36, 39, 34, 35, 32, 33, 25, 26};

//========== VARIABLES GLOBALES ==========
//Peso de la línea a seguir:
int peso[8] = {-5,-3,-1,0,0,1,3,5};

//DECLARACION FUNCIONES
int convertirAColor(int pinAnalogico);

// ----- CONFIG INICIAL -----//
void setup() {
  Serial.begin(115200);
}

// ================= BUCLE PRINCIPAL
void loop() {

  int error = 0;
  int cantLeidos = 0;
  int posicion = 0;
  for(int i=0; i<8; i++){
    //El error es un número que indica si la línea negra, sobre el fondo blanco
    //Está hacía la derecha o izquierda
    posicion += convertirAColor(pinesInfrarrojo[i])* peso[i];
    cantLeidos += convertirAColor(pinesInfrarrojo[i]);
  }
  if (cantLeidos != 0){
    error = posicion/cantLeidos;
  }else{
    Serial.println("Detener motores");
  }
  
  Serial.println(error);
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
