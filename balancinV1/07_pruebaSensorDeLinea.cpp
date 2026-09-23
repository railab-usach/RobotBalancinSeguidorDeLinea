/*  
* El sensor de línea es analógico. Tras unas pruebas, la
  zona blanca marca alrededor de 2500 y en la zona negra 4000.
*/


// =========== PINES ================
int D1 = 36;
int D2 = 39;
int D3 = 34;
int D4 = 35;
int D5 = 32;
int D6 = 33;
int D7 = 25;
int D8 = 36;

// ----- CONFIG INICIAL -----//
void setup() {
  Serial.begin(115200);
}

// ================= BUCLE PRINCIPAL
void loop() {
  Serial.print(analogRead(D1));
  Serial.print(analogRead(D2));
  Serial.print(analogRead(D3));
  Serial.print(analogRead(D4));
  Serial.print(analogRead(D5));
  Serial.print(analogRead(D6));
  Serial.print(analogRead(D7));
  Serial.println(analogRead(D8));
  delay(200);
}
