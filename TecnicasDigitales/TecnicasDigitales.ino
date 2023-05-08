const int Trigger = 14;   //Pin digital 2 para el Trigger del sensor
const int Echo = 12;   //Pin digital 3 para el Echo del sensor
const int Echo2 = 27;
const int Trigger2 = 26;
// const int PIN_RED = 3;
// const int PIN_GREEN = 2;
// const int PIN_BLUE = 1;
int people_count = 0;
 long initDist_1, initDist_2;
 bool isCalibrated = false;
 bool person_passing = false;
void setup() {
  Serial.begin(9600);//iniciailzamos la comunicación
  pinMode(Trigger, OUTPUT); //pin como salida
  pinMode(Echo, INPUT);  //pin como entrada
  digitalWrite(Trigger, LOW);//Inicializamos el pin con 0

  pinMode(Trigger2, OUTPUT);
  pinMode(Echo2, INPUT);
  digitalWrite(Trigger2, LOW);

  calibrate();

  // pinMode(PIN_RED,   OUTPUT);
  // pinMode(PIN_GREEN, OUTPUT);
  // pinMode(PIN_BLUE,  OUTPUT);

  // analogWrite(PIN_RED,   0);
  // analogWrite(PIN_GREEN, 151);
  // analogWrite(PIN_BLUE,  157);
}
void calibrate(){
   initDist_1 = calculateDistance(Trigger, Echo);
   initDist_2 = calculateDistance(Trigger2, Echo2);
   isCalibrated = true;
}

void distance(int trigg, int e){
  // Read distance from Sensor 1
  long distance_1 = calculateDistance(Trigger, Echo);
  Serial.print("dist1 ");
  Serial.println(distance_1);
  
  // Read distance from Sensor 2
  long distance_2 = calculateDistance(Trigger2, Echo2);
  Serial.print("dist2 ");
  Serial.println(distance_2);

  if(distance_1 == initDist_1 && distance_2 == initDist_2){
    person_passing = false;
  }

  if(person_passing){
    return;
  }
  
  // Compare distance values to threshold
  if (distance_1 < initDist_1 - 10){
    Serial.println("leaving");
    person_passing = true;
    people_count--;
  }

  if(distance_2 < initDist_2 - 10){
    person_passing = true;
    people_count++;
  }
  
  // Print people count to serial monitor
  Serial.print("People count: ");
  Serial.println(people_count);
  
  delay(100); // wait a little bit before next reading

}
void regulateLight(int lsens){

}

int calculateDistance(int trigg, int ech){
  digitalWrite(trigg, HIGH);
  delayMicroseconds(10);          //Enviamos un pulso de 10us
  digitalWrite(trigg, LOW);
  long t = pulseIn(ech, HIGH); //obtenemos el ancho del pulso
  return t/59;
}


void loop()
{
  distance(Trigger, Echo);
  distance(Trigger2, Echo2);

}