#include <stdio.h>

int hello_n (int n) {
  while (n > 0) {
	puts("Hello World!");
	n--;
	}
	return 0;
}

int main(int argc, char** argv) {
	hello_n(17);
	return 0;
}