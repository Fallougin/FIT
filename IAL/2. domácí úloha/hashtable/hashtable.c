/*
 * Tabuľka s rozptýlenými položkami
 *
 * S využitím dátových typov zo súboru hashtable.h a pripravených kostier
 * funkcií implementujte tabuľku s rozptýlenými položkami s explicitne
 * zreťazenými synonymami.
 *
 * Pri implementácii uvažujte veľkosť tabuľky HT_SIZE.
 */

#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

int HT_SIZE = MAX_HT_SIZE;

/*
 * Rozptyľovacia funkcia ktorá pridelí zadanému kľúču index z intervalu
 * <0,HT_SIZE-1>. Ideálna rozptyľovacia funkcia by mala rozprestrieť kľúče
 * rovnomerne po všetkých indexoch. Zamyslite sa nad kvalitou zvolenej funkcie.
 */
int get_hash(char *key) {
  int result = 1;
  int length = strlen(key);
  for (int i = 0; i < length; i++) {
    result += key[i];
  }
  return (result % HT_SIZE);
}

/*
 * Inicializácia tabuľky — zavolá sa pred prvým použitím tabuľky.
 */
void ht_init(ht_table_t *table) {
  for (int  i = 0; i < HT_SIZE; i++){ 
    (*table)[i] = NULL;
  }
}

/*
 * Vyhľadanie prvku v tabuľke.
 *
 * V prípade úspechu vráti ukazovateľ na nájdený prvok; v opačnom prípade vráti
 * hodnotu NULL.
 */
ht_item_t *ht_search(ht_table_t *table, char *key) {
  ht_item_t* search = (*table)[get_hash(key)];
  for (; search;search = search->next)
  {
    if (strcmp(key, search->key) == 0)
    {
      return search;
    }
  }
  return NULL;
}

/*
 * Vloženie nového prvku do tabuľky.
 *
 * Pokiaľ prvok s daným kľúčom už v tabuľke existuje, nahraďte jeho hodnotu.
 *
 * Pri implementácii využite funkciu ht_search. Pri vkladaní prvku do zoznamu
 * synonym zvoľte najefektívnejšiu možnosť a vložte prvok na začiatok zoznamu.
 */
void ht_insert(ht_table_t *table, char *key, float value) {
  ht_item_t *search = ht_search(table, key); 
	if (search)
	{
		search->value = value;
    return;
	}

	ht_item_t *item = (ht_item_t *) malloc(sizeof(ht_item_t));
	if (!item)
	{
		return;
	}
	item->key = key;
	item->value = value;
	item->next = NULL;

	int hash = get_hash(key); 
	if ((search = (*table)[hash]))
	{
		item->next = search;
	}

	(*table)[hash] = item;
  
}

/*
 * Získanie hodnoty z tabuľky.
 *
 * V prípade úspechu vráti funkcia ukazovateľ na hodnotu prvku, v opačnom
 * prípade hodnotu NULL.
 *
 * Pri implementácii využite funkciu ht_search.
 */
float *ht_get(ht_table_t *table, char *key) {
  ht_item_t *search = ht_search(table, key); 
  if (search)
  {
    return &(search->value);
  }
  else
  {
    return NULL;  
  }
}

/*
 * Zmazanie prvku z tabuľky.
 *
 * Funkcia korektne uvoľní všetky alokované zdroje priradené k danému prvku.
 * Pokiaľ prvok neexistuje, nerobte nič.
 *
 * Pri implementácii NEVYUŽÍVAJTE funkciu ht_search.
 */
void ht_delete(ht_table_t *table, char *key) {
  ht_item_t *item = (*table)[get_hash(key)];    
  ht_item_t *prevItem = NULL;
  if(item == NULL)
  {	
		return; 
  }
  if(item->key == key) 
  {
    if(item->next == NULL)   
    {   
      (*table)[get_hash(key)] = NULL; 
    }
    else   
    {
      (*table)[get_hash(key)] = item->next;    
    }
    free(item); 
  }
  else    
  {
    while(item != NULL) 
    {  
      if(item->key == key)    
      {
        prevItem->next = item->next;  
        free(item); 
        break;
      }      
      prevItem = item;    
      item = item->next;   
    }
  }
}

/*
 * Zmazanie všetkých prvkov z tabuľky.
 *
 * Funkcia korektne uvoľní všetky alokované zdroje a uvedie tabuľku do stavu po
 * inicializácii.
 */
void ht_delete_all(ht_table_t *table) {
  for(int i = 0; i < HT_SIZE; i++) 
  {
    ht_item_t *item = (*table)[i];
    while(item != NULL)
    {
      ht_item_t *deleteItem = item;
      item = item->next;
      free(deleteItem);
    }
    (*table)[i] = NULL;
  }  
}
