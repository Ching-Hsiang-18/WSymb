void foo() {
  int i;
  for (i = 0 ; i < 10; i++);
}

int main(void) {

  int j;
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
