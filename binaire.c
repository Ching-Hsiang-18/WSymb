int main(void) {
  int i = 0;
  int j; 
  for (i = 0; i < 42; i++) {
    if (i % 2) {
      continue;
    }
    
    if (i % 3) {
      break;
    }
  }

}
