void foo() {
  int i;
  if (i) {
    i++;
  }
}

void bar() {
  int i;
  for (i = 0; i < 10; i++) {
  }
}

int main(void) {
  int i;
  for (i = 0; i < 10; i++) {
    if (i % 2) {
      foo();
    } else {
      bar();
    }
    if (i % 3) {
      continue;
    }
    i += 2;
  }
}
