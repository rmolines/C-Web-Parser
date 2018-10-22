#include <string>
#include <iostream>
#include <cpr/cpr.h>
#include "gumbo.h"
#include <vector>
#include <string>
#include <algorithm>
#include "json.hpp"
#include "semaphore.h"
#include <limits>
#include <time.h> 
#include <fstream>
#include <chrono>
#include <cstdlib>
typedef std::chrono::high_resolution_clock Time;

using json = nlohmann::json;
using namespace std;
bool found = false;
string URL = "https://www.kabum.com.br/computadores/computador-gamer";
bool found_next = false;

int counter = 0;
vector<string> all_urls;
            
vector<json> product_infos;
vector<cpr::Response> rs;

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

static void find_next_page(GumboNode* node, vector<string> &next_urls, bool &found_next){
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
			find_next_page(static_cast<GumboNode*>(children->data[i]), next_urls, found_next);
		}
	}

}

void produceMainPage(Semaphore &s_main_page, Semaphore &s_next_page, vector<cpr::Response> &r_main_pages, vector<string> &all_urls, int &ind_page, bool &has_next, Semaphore &s_stop){
  while(true){ 
    s_next_page.acquire();  
    if (!has_next){
      break;
    }
    r_main_pages.push_back(cpr::Get(cpr::Url{all_urls[ind_page]}));  

    s_main_page.release();
  }


  return;
}

void consumeMainPage(Semaphore &s_main_page, Semaphore &s_next_page, vector<vector<string>> &prod_url_lists, \
          bool &found_next_page, vector<cpr::Response> &r_main_pages, string &main_url, vector<string> &all_urls, int &page_ind, Semaphore &s_prod_url_list, bool &has_next_page, int &num_pages, vector<vector<cpr::Response>> &r_prod\
          , int &num_threads){
    
    string next_url;
    vector<string> url_list_temp;
    std::chrono::duration<double> timer;

    while(true){

      s_main_page.acquire();  
      if (!has_next_page){
        break;
      }

      GumboOutput *output = gumbo_parse(r_main_pages[page_ind].text.c_str());
      find_urls(output->root, next_url, found_next_page, url_list_temp);
      prod_url_lists.push_back(url_list_temp);
      vector<cpr::Response> temp;
      r_prod.push_back(temp);

  
      if(found_next_page){
        all_urls.push_back(main_url+next_url);
        page_ind++;
        found_next_page = false;
      } else {
        has_next_page = false;
        for (int i=0; i<num_threads; i++){
          s_next_page.release();
          s_main_page.release();
        }
        num_pages = page_ind+1;
      }
      
      s_next_page.release();
      
      for(int i=0; i<url_list_temp.size(); i++){
        s_prod_url_list.release();
      }
      url_list_temp.clear();   
    }
  return;
}

void produceProductPage(Semaphore &s_prod_url, int &page_ind, vector<vector<string>> &prod_url_lists, vector<vector<cpr::Response>> &r_prod,\
          Semaphore &s_prod_url_list, int &product_ind, Semaphore &s_prod_ind, int &num_pages, \
          bool &finished, int &num_threads){
  
  int page_ind_temp, prod_ind_temp;
  while(true){

    s_prod_url_list.acquire();

    s_prod_ind.acquire();
    if(finished){
      break;
    }


    if (product_ind+1 >= prod_url_lists[page_ind_temp].size() && page_ind+1>=num_pages){
      finished = true;
      for (int i=0; i<num_threads; i++){
        s_prod_url_list.release();
        s_prod_ind.release();
      }
      break;
    }

    if(product_ind >= prod_url_lists[page_ind_temp].size()){
      product_ind = 0;
      page_ind = page_ind+1;
    }


    prod_ind_temp = product_ind;
    page_ind_temp = page_ind;

    product_ind++;
    cpr::Response temp;
    r_prod[page_ind_temp].push_back(temp);
    s_prod_ind.release();

    r_prod[page_ind_temp][prod_ind_temp] = (cpr::Get(cpr::Url{prod_url_lists[page_ind_temp][prod_ind_temp]}));
    s_prod_url.release();
  }

  return;
}

void consumeProductPage(Semaphore &s_prod_url, vector<vector<cpr::Response>> &r_prod, int &page_ind, int &product_ind, Semaphore &s_html, bool &finished, int &num_pages, int &num_threads, string &category, vector<vector<string>> &prod_url_lists){


  string html, price, f_price, name, description, pic_url;
  std::chrono::duration<double> timer;
  
  while(true){
    bool found = false;
    json j;
    s_prod_url.acquire();
    int prod_temp, page_temp;
    s_html.acquire();
    if (finished){
      break;
    }
    
    if (product_ind+1 >= r_prod[page_ind].size() && page_ind+1>=num_pages){
      finished = true;
      for (int i=0; i<num_threads; i++){
        s_prod_url.release();
        s_html.release();
      }
      break;
    }

    if (product_ind >= r_prod[page_ind].size()){
      product_ind = 0;
      page_ind = page_ind+1;

    }

    html = r_prod[page_ind][product_ind].text;
    page_temp = page_ind;
    prod_temp = product_ind;
    int temp_counter = counter;
    product_ind++;
    counter++;
    s_html.release();

    GumboOutput *output = gumbo_parse(html.c_str());;
    find_product_infos(output->root, found, name, description, pic_url, price, f_price); 
    j["name"] = name+"\n";
    j["description"] = description+"\n";
    j["pic_url"] = pic_url+"\n";
    j["preco a vista"] = price+"\n";
    j["preco parcelado"] = f_price;
    j["categoria"] = category;
    j["url do produto"] = prod_url_lists[page_temp][prod_temp];
    cout << j << endl;
  }

  return;
}



static void create_jsons_parallel(vector<string> &all_urls, bool &found_next, std::ofstream &myfile, int num_threads){

  Semaphore s_main_page(0), s_next_page(1), s_next(1), s_prod(1), s_url_list(1), s_prod_url_list(0), s_prod_url(0), s_prod_ind(1), s_html(1), s_stop(1);
  bool has_next_page = true;//always starts true, there is always first page
  vector<cpr::Response> r_main_pages;
  vector<vector<string>> main_url_lists, prod_url_lists;
  vector<vector<cpr::Response>> r_prod;
  bool found_next_page = false, finished = false, finished_consumption = false, finished_production = false;
  string main_url = URL;
  int num_pages = numeric_limits<int>::max(), product_ind_prod = 0, page_ind_prod = 0, page_ind_cons = 0, product_ind_cons = 0, page_ind = 0, product_ind = 0;
  vector<pair<pair<int, int>, float>> ttpp;//tempo total por produto
  vector<float> time_down;
  
  auto pos = URL.find_last_of('/');
  string category = URL.substr(pos+1);
  string next_url = URL;
  all_urls.push_back(URL);
  int sp = 0, counter=0;
  vector<thread> t;

  Time::time_point tto1 = Time::now();//tempo total ocioso
  
  Time::time_point tto2;


  for (int i=0; i<num_threads; i++){
    t.push_back(thread(produceMainPage, ref(s_main_page), ref(s_next_page), ref(r_main_pages), ref(all_urls), ref(page_ind), ref(has_next_page), ref(s_stop)));

    t.push_back(thread(consumeMainPage, ref(s_main_page), ref(s_next_page), ref(prod_url_lists), ref(found_next_page), ref(r_main_pages), ref(main_url), ref(all_urls), ref(page_ind), ref(s_prod_url_list), ref(has_next_page), ref(num_pages), ref(r_prod), ref(num_threads)));

    t.push_back(thread(produceProductPage, ref(s_prod_url), ref(page_ind_prod), ref(prod_url_lists), ref(r_prod), ref(s_prod_url_list), ref(product_ind_prod), ref(s_prod_ind), ref(num_pages), ref(finished_production), ref(num_threads)));

    t.push_back(thread(consumeProductPage, ref(s_prod_url), ref(r_prod), ref(page_ind_cons), ref(product_ind_cons), ref(s_html), ref(finished_consumption), ref(num_pages), ref(num_threads), ref(category), ref(prod_url_lists)));
  }

  for (int i=0; i<t.size(); i++){
    t[i].join();
  }

  float temp = 0;
  for(int i=0; i<time_down.size(); i++){
    temp += time_down[i];
  }

  // myfile << temp << endl;
}


int main(int argc, char ** argv)
{ 
  int num_threads;
  if (argv[1]){
    num_threads = atoi(argv[1]);;
  } else {
    num_threads = 16;
  }

  string temp;
  if (argv[2]){
    temp = argv[2];
  } else {
    temp = URL;
  }
  
  
  URL = temp;
  Time::time_point t1 = Time::now();

  std::string filename = "./par_time.txt";
  std::ofstream myfile;
  myfile.open (filename);
   
  create_jsons_parallel(all_urls, found_next, myfile, num_threads);
  Time::time_point t2 = Time::now();
  std::chrono::duration<double> timer;
  timer = std::chrono::duration_cast<std::chrono::duration<double>> (t2-t1);
  cout << "Tempo total: " << timer.count() << "s" << endl;
  myfile << timer.count() << endl;

  
  myfile.close();
  
  return 0;
}
