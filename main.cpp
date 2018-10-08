#include <string>
#include <iostream>
#include <cpr/cpr.h>
// #include "gumbo.h"

using namespace std;

string URL = "https://www.kabum.com.br/computadores/computador-gamer";

auto r = cpr::Get(cpr::Url{URL});

int main()
{
  cout << r.text << endl;
  // GumboOutput *output = gumbo_parse("<h1>Hello, World!</h1>");
  // // Do stuff with output->root
  // gumbo_destroy_output(&kGumboDefaultOptions, output);
}
