#include <string>
#include <iostream>
#include <cpr/cpr.h>
#include "gumbo.h"
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

string URL = "https://www.kabum.com.br/produto/96291/computador-gamer-zotac-intel-core-i7-7700-16gb-hd-1tb-ssd-240gb-gtx-1070ti-bluetooth-windows-10-home-g1107tk700b-u";

auto r = cpr::Get(cpr::Url{URL});

int counter = 0;
vector<string> url_list, product_names, product_descriptions, \
              pic_urls, view_prices, financed_priced, product_categories;

static void search_for_links(GumboNode* node) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* href;
  if (node->v.element.tag == GUMBO_TAG_A &&
      (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
    
    if (href->value[25] == 'p'){
      if(find(url_list.begin(), url_list.end(), (href->value)) == url_list.end()){
        url_list.push_back(href->value);
        // std::cout << href->value << std::endl;
        counter++;
      }
    }
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links(static_cast<GumboNode*>(children->data[i]));
  }
}

static void find_email(GumboNode* node){
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

static void find_product_infos(GumboNode* node){
	if (node->type == GUMBO_NODE_ELEMENT)
	{
    GumboAttribute *vAttribute;

    //PRODUCT NAME
		if (node->v.element.tag == GUMBO_TAG_H1){
      if (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "itemprop")){
        if (strcmp(vAttribute->value, "name") == 0){
          product_names.push_back(vAttribute->value);
          
          GumboNode* title_text = static_cast<GumboNode *>(node->v.element.children.data[0]);
          if (title_text->type == GUMBO_NODE_TEXT)
          {
            product_descriptions.push_back(title_text->v.text.text);
          }
        }
      }
    } 

    //PRODUCT DESCRIPTION
		else if (node->v.element.tag == GUMBO_TAG_P){
      if (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "itemprop")){
        if (strcmp(vAttribute->value, "description") == 0){
          
          GumboNode* title_text = static_cast<GumboNode *>(node->v.element.children.data[0]);
          if (title_text->type == GUMBO_NODE_TEXT)
          {
            product_descriptions.push_back(title_text->v.text.text);
          }
        }
      }
    }

    //PIC URL
		else if (node->v.element.tag == GUMBO_TAG_DIV){
      if (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "class")){
        cout << vAttribute->value << endl;
        if (strcmp(vAttribute->value, "img-zoom-container") == 0){

          GumboNode* img_url = static_cast<GumboNode *>(node->v.element.children.data[0]);
          if (img_url->type == GUMBO_NODE_ELEMENT)
          { 
            if (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "class")){
              cout << vAttribute->value << endl;
            }

            product_descriptions.push_back(img_url->v.text.text);
          }
        }
      }
    }

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
      find_product_infos(static_cast<GumboNode*>(children->data[i]));
    }
  }
}



int main()
{
  // cout << r.text.c_str() << endl;
  GumboOutput *output = gumbo_parse(r.text.c_str());
  // // Do stuff with output->root
  // find_email(output->root);
  find_product_infos(output->root);
  gumbo_destroy_output(&kGumboDefaultOptions, output);
}
