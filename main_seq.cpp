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
typedef std::chrono::high_resolution_clock Time;


// for convenience
using json = nlohmann::json;
using namespace std;
bool found = false;
string URL = "https://www.kabum.com.br/computadores/computador-gamer";
bool found_next;
auto r = cpr::Get(cpr::Url{URL});

int counter = 0;
vector<string> url_list, product_names, product_descriptions, \
              pic_urls, view_prices, financed_priced, product_categories, next_urls, all_urls;
            
vector<json> product_infos;

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

static void find_next_page(GumboNode* node){
	if (node->type == GUMBO_NODE_ELEMENT)
	{
    GumboAttribute *vAttribute;
		if (node->v.element.tag == GUMBO_TAG_A && 
        (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "target"))) {
        if (strcmp(vAttribute->value, "_top") == 0){
          
          GumboNode* title_text = static_cast<GumboNode *>(node->v.element.children.data[0]);
          if (title_text->type == GUMBO_NODE_TEXT)
          {
            if (strcmp(title_text->v.text.text, "Proxima > ") == 0){
              vAttribute = gumbo_get_attribute(&node->v.element.attributes, "href");
              if (!found_next){
                next_urls.push_back(vAttribute->value);
                found_next = true;
              }
            }
          }
        }
		}

		GumboVector* children = &node->v.element.children;

		for (unsigned int i = 0; i < children->length; ++i)
		{
			find_next_page(static_cast<GumboNode*>(children->data[i]));
		}
	}

}

static void find_all_pages(){
  GumboOutput *output = gumbo_parse(r.text.c_str());

  string next_url;
  while(!found_next){
  }
  gumbo_destroy_output(&kGumboDefaultOptions, output);

  cout << "done with next page\n";
}

static void create_jsons(){
  float ttd=0.0f, tta=0.0f; //tempo total download, tempo total analise

  Time::time_point t1 = Time::now();    
  GumboOutput *output = gumbo_parse(r.text.c_str());
  Time::time_point t2 = Time::now();    
  tta += std::chrono::duration_cast<std::chrono::duration<double>> (t2-t1).count();

  auto pos = URL.find_last_of('/');
  string category = URL.substr(pos+1);
  string next_url;
  all_urls.push_back(URL);
  
  while (!found_next){
    
    t1 = Time::now();    
    auto r = cpr::Get(cpr::Url{all_urls[all_urls.size()-1]});
    t2 = Time::now();    
    ttd += std::chrono::duration_cast<std::chrono::duration<double>> (t2-t1).count();

    output = gumbo_parse(r.text.c_str());   
    find_urls(output->root); 
    for (int i=0; i<url_list.size(); i++){
      product_categories.push_back(category);

      t1 = Time::now();    
      auto r = cpr::Get(cpr::Url{url_list[i]});
      t2 = Time::now();    
      ttd += std::chrono::duration_cast<std::chrono::duration<double>> (t2-t1).count();

      t1 = Time::now();    
      GumboOutput *output = gumbo_parse(r.text.c_str());
      json product;
      product = find_product_infos(output->root, product); 
      cout << product << endl; 
      t2 = Time::now();    
      tta += std::chrono::duration_cast<std::chrono::duration<double>> (t2-t1).count();  
      
      cout << i << endl;
      found = false;
      // cout << '.';
      cout.flush();
    }
    url_list.clear();


    find_next_page(output->root);
    if (found_next){
      found_next = false;
      next_url = URL+next_urls[next_urls.size()-1];
      all_urls.push_back(next_url);
      
      t1 = Time::now();    
      auto r2 = cpr::Get(cpr::Url{next_url});
      t2 = Time::now();    
      ttd += std::chrono::duration_cast<std::chrono::duration<double>> (t2-t1).count();

      t1 = Time::now();    
      output = gumbo_parse(r2.text.c_str());
      t2 = Time::now();    
      tta += std::chrono::duration_cast<std::chrono::duration<double>> (t2-t1).count();
    } else {
      found_next = true;
    }
  }
  gumbo_destroy_output(&kGumboDefaultOptions, output);

  cout << "tta: " << tta << " ttd: " << ttd << endl;

}


int main(int argc, char ** argv)
{

  string temp = argv[1];
  URL = temp;

  std::string filename = "./seq_time.txt";
  std::ofstream myfile;
  myfile.open (filename);

  Time::time_point t1 = Time::now();    
  create_jsons();
  Time::time_point t2 = Time::now();    
  std::chrono::duration<double> timer = std::chrono::duration_cast<std::chrono::duration<double>> (t2-t1);
  cout << "tt: " << timer.count() << endl;
  myfile << timer.count() << endl;

  
  myfile.close();

}
