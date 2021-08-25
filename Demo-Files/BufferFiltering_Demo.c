// C++ program to implement
// the above approach
 
#include <stdio.h>
#define BUFFER_SIZE 5

 
typedef struct {
  char address[17];   // 67:f1:d2:04:cd:5d
  int rssi;
} BeaconData;

void printBuffer(BeaconData *buffer,int bufferSize);
void findUniqueInBuffer(BeaconData *buffer,int bufferSize);

int main()
{
 	BeaconData myBuffer[BUFFER_SIZE];
 	
 	strcpy (myBuffer[0].address, "01:1B:44:11:3A:B7");
 	strcpy (myBuffer[1].address, "02:1B:44:12:3A:B7");
 	strcpy (myBuffer[2].address, "02:1B:44:12:3A:B7");
 	strcpy (myBuffer[3].address, "01:1B:44:11:3A:B7");
 	strcpy (myBuffer[4].address, "03:1B:44:14:3A:B7");
 	
 	myBuffer[0].rssi = 10; 
	myBuffer[1].rssi = 10; 
	myBuffer[2].rssi = 10; 
	myBuffer[3].rssi = 10; 
	myBuffer[4].rssi = 10;
	
	printBuffer(myBuffer,BUFFER_SIZE);
	findUniqueInBuffer(myBuffer,BUFFER_SIZE);
 	
    return 0;
}

void printBuffer(BeaconData *buffer,int bufferSize) {
	int i = 0;
	for(i = 0; i<bufferSize;i++) {
 		printf("%d : %s : %d\n",i,buffer[i].address,buffer[i].rssi);
	 }
}

void findUniqueInBuffer(BeaconData *buffer,int bufferSize)  {

BeaconData uniqueBuffer[bufferSize];

int modelCounted[5] = {0};
int numberOfUniqeAdresses = 0;
int currentRssi = 0;
printf("\n");
int i  = 0; int j = 0; int counter = 0;
  for (j = 0; j < bufferSize; j++) {
    if (!modelCounted[j])  // Only enter inner-loop if car hasn't been counted already
    {
      for (i = 0; i < bufferSize; i++) {
        if (strcmp(buffer[i].address, buffer[j].address) == 0) {
          counter++;
          currentRssi += buffer[i].rssi;
          modelCounted[i] = 1;  // Mark car as counted
        }
      }
      numberOfUniqeAdresses++;
      printf("%s = %d\n", buffer[j].address, counter);
      strcpy(uniqueBuffer[numberOfUniqeAdresses-1].address,buffer[j].address);
      uniqueBuffer[numberOfUniqeAdresses-1].rssi = currentRssi/counter;
      counter = 0; currentRssi = 0;
    }
  }
  printf("\n");
printf("Devices Found: %d\n",numberOfUniqeAdresses);
  printBuffer(uniqueBuffer,numberOfUniqeAdresses);
	
}
