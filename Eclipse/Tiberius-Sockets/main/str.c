/*
 * str.c
 *
 *  Created on: 26 jul. 2020
 *      Author: Renato
 */

/*
 * src: inicio
 * dst: final
 * in: string donde buscar
 * out: sting encontrado
 * */
#include "main.h"

void String_Between(char* in,char* start, char* end,  char* out){
	char *searchStart;
	char *searchEnd;

	if(in == NULL) 
		return;
	
	if(strcmp(start,"") == 0) searchStart = in;
	else 					  searchStart = strstr(in,start);

	if(searchStart == NULL)
		return;

	searchStart = searchStart + strlen(start);


	if(strcmp(end,"") == 0)  searchEnd = searchStart + strlen(searchStart);
	else					 searchEnd = strstr(searchStart,end);

	if(searchEnd == NULL)
		return;

	strlcpy(out,searchStart,searchEnd - searchStart + 1);
}


void String_Remplace(char *src,char*name, char *line)
{
	char *token;
	char result[256] = {0};

	if( strcmp(src,"")==0 )
		return;

   token = strtok(src, "\r\n");

   while( token != NULL )
   {
	  if(strstr(token, name)!=NULL)
		strcat(result, line);
	  else
		strcat(result, token);

      token = strtok(NULL, src);
   }

   strcpy(src, result);
}

bool String_Is_Empy(char * str){
	if(strcmp(str,"")==0) return 1;
	else return 0;
}



