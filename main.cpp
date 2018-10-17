#include <string>
#include <iostream>
#include <cpr/cpr.h>
#include "gumbo.h"
#include <vector>
#include <string>
#include <algorithm>
#include "json.hpp"

// for convenience
using json = nlohmann::json;
using namespace std;
bool found = false;
string URL = "https://www.kabum.com.br/computadores/computador-gamer";

auto r = cpr::Get(cpr::Url{URL});

int counter = 0;
vector<string> url_list, product_names, product_descriptions, \
              pic_urls, view_prices, financed_priced, product_categories;
            
vector<json> product_infos;

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
        counter++;
      }
    }
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links(static_cast<GumboNode*>(children->data[i]));
  }
}

static void find_urls(GumboNode* node){
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

		}

		GumboVector* children = &node->v.element.children;

		for (unsigned int i = 0; i < children->length; ++i)
		{
			find_urls(static_cast<GumboNode*>(children->data[i]));
		}
	}
}

static json find_product_infos(GumboNode* node, json j){
	if (node->type == GUMBO_NODE_ELEMENT)
	{
    GumboAttribute *vAttribute;

    //PRODUCT NAME
		if (node->v.element.tag == GUMBO_TAG_H1){
      if (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "itemprop")){
        if (strcmp(vAttribute->value, "name") == 0){
          // product_names.push_back(vAttribute->value);
          
          GumboNode* title_text = static_cast<GumboNode *>(node->v.element.children.data[0]);
          if (title_text->type == GUMBO_NODE_TEXT)
          {
            j["name"] = title_text->v.text.text;
            product_names.push_back(title_text->v.text.text);
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
            j["description"] = title_text->v.text.text;
            product_descriptions.push_back(title_text->v.text.text);
          }
        }
      }
    }

    //FINANCED PRICE
		else if (node->v.element.tag == GUMBO_TAG_DIV){
      if (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "class")){
        if (strcmp(vAttribute->value, "preco_normal") == 0){
          
          GumboNode* title_text = static_cast<GumboNode *>(node->v.element.children.data[0]);
          if (title_text->type == GUMBO_NODE_TEXT)
          {
            j["f_price"] = title_text->v.text.text;

            financed_priced.push_back(title_text->v.text.text);
          }
        }
      }
    }

    //VIEW PRICE
    else if (node->v.element.tag == GUMBO_TAG_SPAN){
      if (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "itemprop")){
        if (strcmp(vAttribute->value, "offers") == 0){
          GumboVector* children = &node->v.element.children;
          for (unsigned int i = 0; i < children->length; ++i) {
            GumboNode *node = static_cast<GumboNode*>(children->data[i]);
            if (node->type == GUMBO_NODE_ELEMENT)
            {
              GumboAttribute *vAttribute;
              if (node->v.element.tag == GUMBO_TAG_STRONG){
                GumboNode* title_text = static_cast<GumboNode *>(node->v.element.children.data[0]);
                if (title_text->type == GUMBO_NODE_TEXT)
                {
                  j["price"] = title_text->v.text.text;

                  view_prices.push_back(title_text->v.text.text);
                }
              }                      
            }
          }        
        }        
      }
    }

    //PIC URL
		else if (node->v.element.tag == GUMBO_TAG_IMG){
      if (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "itemprop")){
        // cout << vAttribute->value << endl;
        if (strcmp(vAttribute->value, "image") == 0){
          GumboAttribute* url = gumbo_get_attribute(&node->v.element.attributes, "src");
          string url_value = url->value;
          if (!found){
            j["pic_url"] = url_value;
            
            found = true;
          }
        
        }
      }
    }

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
      j = find_product_infos(static_cast<GumboNode*>(children->data[i]), j);
    }
  }
  return j;
}



int main()
{
  GumboOutput *output = gumbo_parse(r.text.c_str());

  find_urls(output->root);
  auto pos = URL.find_last_of('/');
  string category = URL.substr(pos+1);

  for (int i=0; i<url_list.size(); i++){
    product_categories.push_back(category);
    auto r = cpr::Get(cpr::Url{url_list[i]});
    GumboOutput *output = gumbo_parse(r.text.c_str());
    json product;
    product = find_product_infos(output->root, product);    
    cout << product << endl;
    found = false;
    // cout << '.';
    cout.flush();
  }
  cout << endl;
  cout << product_descriptions.size() << " " << product_names.size() << " " << product_categories.size() << " " \
          << financed_priced.size() << " " << view_prices.size() << " " << pic_urls.size() << endl;
  gumbo_destroy_output(&kGumboDefaultOptions, output);
}
