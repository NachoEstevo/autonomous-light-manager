const int Trigger = 14;   //Pin digital 2 para el Trigger del sensor
const int Echo = 12;   //Pin digital 3 para el Echo del sensor
const int Echo2 = 27;
const int Trigger2 = 26;
// const int PIN_RED = 3;
// const int PIN_GREEN = 2;
// const int PIN_BLUE = 1;
void setup() {
  Serial.begin(9600);//iniciailzamos la comunicación
  pinMode(Trigger, OUTPUT); //pin como salida
  pinMode(Echo, INPUT);  //pin como entrada
  digitalWrite(Trigger, LOW);//Inicializamos el pin con 0

  pinMode(Trigger2, OUTPUT);
  pinMode(Echo2, INPUT);
  digitalWrite(Trigger2, LOW);

  // pinMode(PIN_RED,   OUTPUT);
  // pinMode(PIN_GREEN, OUTPUT);
  // pinMode(PIN_BLUE,  OUTPUT);

  // analogWrite(PIN_RED,   0);
  // analogWrite(PIN_GREEN, 151);
  // analogWrite(PIN_BLUE,  157);
}

void distance(int trigg, int e){
  long t;
  long d;

  digitalWrite(trigg, HIGH);
  delayMicroseconds(10);          //Enviamos un pulso de 10us
  digitalWrite(trigg, LOW);
  t = pulseIn(e, HIGH); //obtenemos el ancho del pulso
  d = t/59;
  Serial.print("Distancia ");
  Serial.print(trigg);
  Serial.print(": ");
  Serial.print(d);      //Enviamos serialmente el valor de la distancia
  Serial.print("cm");
  Serial.println();
  
  delay(100); 
}


void loop()
{
  distance(Trigger, Echo);
  distance(Trigger2, Echo2);

}