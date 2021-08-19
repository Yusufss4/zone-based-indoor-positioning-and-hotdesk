
#define NUMBER_OF_STRING 4
#define MAX_STRING_SIZE 40

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println("Waiting for the readBytes");
  // Get next command from Serial (add 1 for final 0)
  char input[MAX_STRING_SIZE + 1];
  strcpy(input, "1:90;2:80;3:180");
  char stringArray[NUMBER_OF_STRING][MAX_STRING_SIZE + 1];

  // Read each command pair
  char* command = strtok(input, ";");
  int i = 0;
  while (command != NULL)
  {
    // Split the command in two values
    strcpy(stringArray[i], command);
    command = strtok(0, ";");
    i++;
  }
  // Wait for two seconds (to demonstrate the active low LED)
  Serial.println(stringArray[0]);
  Serial.println(stringArray[1]);
  Serial.println(stringArray[2]);
}

void stringTokenizer(char *stringArray, char* inputString) {
  //do something
}
