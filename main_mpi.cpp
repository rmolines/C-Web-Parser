#include <string>
#include <iostream>
#include <cpr/cpr.h>
#include "gumbo.h"
#include <vector>
#include <string>
#include <algorithm>
#include "json.hpp"
#include <chrono>
#include <fstream> 
#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>



// for convenience
using json = nlohmann::json;
using namespace std;
string URL = "https://www.kabum.com.br/computadores/computador-gamer";

namespace mpi = boost::mpi;
boost::mpi::environment env;
boost::mpi::communicator world;


static void search_for_links(GumboNode* node, vector<string> &url_list) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* vAttribute;
  if (node->v.element.tag == GUMBO_TAG_A &&
      (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
    
    if (vAttribute->value[25] == 'p'){
      if(find(url_list.begin(), url_list.end(), (vAttribute->value)) == url_list.end()){
        url_list.push_back(vAttribute->value);
      }
    }
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links(static_cast<GumboNode*>(children->data[i]), url_list);
  }
}

static void find_urls(GumboNode* node, string &next_url, bool &found_next, vector<string> &url_list){
	if (node->type == GUMBO_NODE_ELEMENT)
	{
    GumboAttribute *vAttribute;
		if (node->v.element.tag == GUMBO_TAG_DIV && 
        (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "id"))) {


      if (strcmp(vAttribute->value, "BlocoConteudo") == 0){
        GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
          search_for_links(static_cast<GumboNode*>(children->data[i]), url_list);
        }        
  
      }
		}
    else if (node->v.element.tag == GUMBO_TAG_A && 
        (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "target"))) {
      if (strcmp(vAttribute->value, "_top") == 0){
        
        GumboNode* title_text = static_cast<GumboNode *>(node->v.element.children.data[0]);
        if (title_text->type == GUMBO_NODE_TEXT)
        {
          if (strcmp(title_text->v.text.text, "Proxima > ") == 0){
            vAttribute = gumbo_get_attribute(&node->v.element.attributes, "href");
            if (!found_next){
              next_url = (vAttribute->value);
              found_next = true;
            }
          }
        }
      }
    }

		GumboVector* children = &node->v.element.children;

		for (unsigned int i = 0; i < children->length; ++i)
		{
			find_urls(static_cast<GumboNode*>(children->data[i]), next_url, found_next, url_list);
		}
	}
}

static void find_product_infos(GumboNode* node, bool &found, string &name, string &description, string &pic_url,\
             string &price, string &f_price){
	if (node->type == GUMBO_NODE_ELEMENT)
	{
    GumboAttribute *vAttribute;

    //PRODUCT NAME
		if (node->v.element.tag == GUMBO_TAG_H1){
      if (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "itemprop")){
        if (strcmp(vAttribute->value, "name") == 0){
          
          GumboNode* title_text = static_cast<GumboNode *>(node->v.element.children.data[0]);
          if (title_text->type == GUMBO_NODE_TEXT)
          {
            name = title_text->v.text.text;
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
            description = title_text->v.text.text;
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
           f_price = title_text->v.text.text;
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
                  price = title_text->v.text.text;
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
        if (strcmp(vAttribute->value, "image") == 0){
          GumboAttribute* url = gumbo_get_attribute(&node->v.element.attributes, "src");
          string url_value = url->value;
          if (!found){
            pic_url = url_value;
            
            found = true;
          }
        
        }
      }
    }

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
      find_product_infos(static_cast<GumboNode*>(children->data[i]), found, name, description, pic_url, \
              price, f_price);
    }
  }
}


static void create_jsons(){


    
  if (world.rank() == 0){
    vector<vector<string>> prod_urls;
    bool found_next = true;
    vector<string> url_list;
    string next_url = URL;
    GumboOutput *output;
    int n_prods = 0;
    int planet;
    vector<int> prods_per_process;

    auto r = cpr::Get(cpr::Url{next_url});

    output = gumbo_parse(r.text.c_str());

    auto pos = URL.find_last_of('/');
    string category = URL.substr(pos+1);
  
    for (int i=1; i<world.size(); i++){
      prods_per_process.push_back(0);
    }

    while(found_next){
      auto r = cpr::Get(cpr::Url{next_url});
      output = gumbo_parse(r.text.c_str());   
      find_urls(output->root, next_url, found_next, url_list); 

      planet = 1;

      for (int i=0; i<url_list.size(); i++){
        world.send(planet, 0, url_list[i]);
        prods_per_process[planet]++;
        if (planet++ >= world.size()){
          planet = 1;
        }
        n_prods++;
      }
      next_url = URL + next_url;
      url_list.clear();
    }

    cout << "Numero de produtos: " << n_prods << endl;
    vector<string> infos;
    for (int i=0; i<prods_per_process.size(); i++){
      for (int j=0; j<prods_per_process[i]; i++){
        world.recv(i, 1, infos);
        for (int k=0; k<infos.size(); k++){
          cout << "INFOS: " << infos[k] << endl;
        }
        infos.clear();
      }
    }

  }
  else {

    string prod_url;
    GumboOutput *output;
    while (true){
      world.recv(0, 0, prod_url);

      if(prod_url.compare("done") == 0){
        break;
      }
      else {
        string name, description, pic_url, price, f_price;
        bool found = false;
        json j;
        auto r = cpr::Get(cpr::Url{prod_url});
        output = gumbo_parse(r.text.c_str()); 
        find_product_infos(output->root, found, name, description, pic_url, price, f_price);

                      

        vector<string> infos;
        infos.push_back(name);
        infos.push_back(description);
        infos.push_back(price);


        world.send(0, 1, infos);
      }
    }
    gumbo_destroy_output(&kGumboDefaultOptions, output);

  }
}


int main(int argc, char ** argv)
{

  string temp = argv[1];
  // URL = temp;


  create_jsons();

}
