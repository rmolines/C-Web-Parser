#include <string>
#include <iostream>
#include <cpr/cpr.h>
#include "gumbo.h"

using namespace std;

string URL = "https://www.kabum.com.br/computadores/computador-gamer";

auto r = cpr::Get(cpr::Url{URL});

static void search_for_links(GumboNode* node) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* href;
  if (node->v.element.tag == GUMBO_TAG_A &&
      (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
    
    if (href->value[25] == 'p'){
      std::cout << href->value << std::endl;
    }
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links(static_cast<GumboNode*>(children->data[i]));
  }
}

static void find_email(GumboNode* node)
{
	if (node->type == GUMBO_NODE_ELEMENT)
	{
    GumboAttribute *vAttribute;
		if (node->v.element.tag == GUMBO_TAG_DIV && 
        (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "id"))) {

      // cout << vAttribute->value << endl << strcmp(vAttribute->value, "BlocoConteudo") << endl;

      if (strcmp(vAttribute->value, "BlocoConteudo") == 0){
        GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
          search_for_links(static_cast<GumboNode*>(children->data[i]));
        }
        
  
      }

			// GumboVector* dt_children = &node->v.element.children;
			// GumboNode* dt_first_child = static_cast<GumboNode*>(dt_children->data[0]);

			// if (dt_first_child->type == GUMBO_NODE_TEXT && string(dt_first_child->v.text.text).compare("Email Address") == 0)
			// {
			// 	printf("FOUND EMAIL ADDRESS");
			// }
		}

		GumboVector* children = &node->v.element.children;

		for (unsigned int i = 0; i < children->length; ++i)
		{
			find_email(static_cast<GumboNode*>(children->data[i]));
		}
	}
}



int main()
{
  // cout << r.text.c_str() << endl;
  GumboOutput *output = gumbo_parse(r.text.c_str());
  // // Do stuff with output->root
  find_email(output->root);
  gumbo_destroy_output(&kGumboDefaultOptions, output);
}
