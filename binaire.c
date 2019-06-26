int main(void) {
  int i = 0;
  int j; 
  for (j = 0; j < 10; j++) {
  for (i = 0; i < 10; i++) {
  }
  for (i = 0; i < 10; i++) {
    if (i % 2) {
      continue;
    }
    
    if (i % 3) {
      break;
    }
    
  }

  }

  while(1) {
    i++;    
    if ((i % 7 ) || (i % 6)) break;
//    if ((i % 7 )) break;
    if ((i % 2) || (i % 3)) {
      continue;
    }
    
}


i++;
if (i % 3) {
  i++;
}  else {
  i--;
}
}
/*
void main(void) {
  int i;
  for (i = 0; i < 10; i++)
    toto();
}
*/