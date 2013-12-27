// RUN: %clang_cc1 -std=gnu++98 -fsyntax-only -Wno-unused-value -verify %s

int main() {
  []{}; // expected-error {{expected expression}}
}
