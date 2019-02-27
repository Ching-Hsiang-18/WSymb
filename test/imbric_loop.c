void _start(void) {
  int i;
  int j;
  int k;

  k++;

  for (i = 0; i < 10; i++) {
    for (j = 0; j < 10 ; j++){
      if (k > 5){
        k--;
      }
      else{
        k++;
      }
    }
  }
  k++;
}
