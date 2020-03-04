void foo() {
  int i;
  int k;
  if (k) {
    for (i = 0 ; i < 10; i++);
  } else for (i = 0; i < 10; i++);
}

int main(void) {

  int j;
  int k;
  for (j = 0; j < 10; j++)
  foo();

  foo();
  
  int i;  
  if (i) {
   i++;
  } else {
   i--;
  }
}
