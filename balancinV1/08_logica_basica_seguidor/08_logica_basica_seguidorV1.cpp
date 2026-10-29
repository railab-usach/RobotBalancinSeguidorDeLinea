/*  
* FUNCIÓN: El siguiente código es para crear el algorítmo más básico de un
  seguidor de linea. No se busca que busque la linea al salirse de la pista
  ni que se detenga en un fondo completamente blanco o negro.
*/


//######################################################################
//##############################  PINES  ###############################
//######################################################################
int pinesInfrarrojo[8] = {36, 39, 34, 35, 32, 33, 25, 26};

//######################################################################
//#######################  VARIABLES GLOBALES  #########################
//######################################################################
//Peso de la línea a seguir:
int peso[8] = {-5,-3,-1,0,0,1,3,5};

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

  int error, cantLeidos, posicion = 0;
  for(int i=0; i<8; i++){
    posicion += convertirAColor(pinesInfrarrojo[i])* peso[i];
    cantLeidos += convertirAColor(pinesInfrarrojo[i]);
  }
  if (cantLeidos != 0){
    //El error es un número que indica si la línea está hacía la derecha o izquierda
    error = posicion/cantLeidos;
  }else{
    Serial.println("Detener motores");
  }
  
  Serial.println(error);
  delay(200);
}

//######################################################################
//#####################  DEFINICION DE FUNCIONES  ######################
//######################################################################
int convertirAColor(int pinAnalogico){
  if(analogRead(pinAnalogico) > 3500){
    return 1; //Color negro
  }
  else{
    return 0; //Color blanco
  }
}
