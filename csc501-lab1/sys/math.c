#include <stdio.h>
#include <math.h>
#define RAND_MAX  0x7FFF

double pow(double x, int y){
  int i;
  double result = x;
  if(y == 0){
    return 1;
  }
  for(i = 1; i < y; i++){
    result *= x;
  }
  return result;
}

double log(double x){
  int i;
  double result = 0;
  for(i = 1; i <= 20; i++){     //Taylor Series Expansion to 20 steps to approx ln(x)
    result += pow(-1,i-1)*pow(x-1,i)/((double)i);
  }
  return result;
}

double expdev(double lambda) {
  double dummy;
  do
    dummy = (double) rand() / RAND_MAX;
    while (dummy == 0.0);
  return -log(dummy) / lambda;
}
