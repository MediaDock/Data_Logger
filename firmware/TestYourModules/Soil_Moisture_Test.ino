#define SensorPIN A0 // pin of moisture sensor
int PowerPIN = 7;
const int dry = 595; // value for dry sensor
const int wet = 239; // value for wet sensor


void setup() {
  Serial.begin(9600);
  pinMode(PowerPIN, OUTPUT);  
}

void loop()
{
  digitalWrite(PowerPIN, HIGH);
  delay(100);
  int sensorVal = analogRead(A0);
  int percentageHumididy = map(sensorVal, wet, dry, 100, 0); 

  Serial.print(sensorVal);
  Serial.print(" ");
  Serial.print(percentageHumididy);
  Serial.println("%");

  delay(100);
  digitalWrite(PowerPIN, LOW);
  delay(500);
}