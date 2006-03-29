/* $Id$ */
%{
	#define _GNU_SOURCE
	#include <string.h>
	#include <stdio.h>
	#include <memory.h>
	#include <errno.h>
	
	#include "parser.h"

	inline void string_reset();
	inline char *string_get_buffer();
	inline int string_get_len();

	extern char *yytext;
	extern FILE *yyin;

	static struct sc_shellcode *shellcodes = NULL;
	extern int line_number;

	static struct sc_shellcode *init_shellcode();
	
	static char error_buffer[0xff];
%}


%token SC_ID SC_LPAR SC_RPAR SC_LBR SC_RBR SC_COMMA SC_SEMI SC_COLON SC_NONE SC_FLAGS SC_PATTERN SC_TYPE SC_MAPPING SC_STRING 
SC_XOR SC_LINKXOR SC_KONSTANZXOR SC_LEIMBACHXOR 
SC_BIND_SHELL 
SC_CONNECTBACK_SHELL 
SC_CONNECTBACK_FILETRANSFER SC_BIND_FILETRANSFER
SC_EXECUTE 
SC_DOWNLOAD 
SC_URL 
SC_BASE64
SC_KEY SC_SUBKEY SC_SIZE SC_SIZEINVERT SC_HOST SC_PORT SC_COMMAND
SC_URI
SC_PCRE SC_PRELOAD SC_POSTLOAD
SC_HOSTKEY SC_PORTKEY

%start body

%%

body
	: /* \epsilon */
	| body shellcode
	;

shellcode
	: identifier SC_LBR statements SC_RBR SC_SEMI
	{
		int i;
/*		
		printf("shellcode:\n");

		printf("\tname               %s\n", shellcodes->name);
		printf("\tnamespace          %s (%d) \n", sc_get_namespace_by_numeric(shellcodes->nspace), shellcodes->nspace);
//		printf("\tpattern            %s\n", shellcodes->pattern);
		printf("\tmap-size           %d\n", shellcodes->map_items);
		printf("\tmap                ");

		for( i = 0; i < shellcodes->map_items; i++ )
		{
			printf("%s (%d) ", sc_get_mapping_by_numeric(shellcodes->map[i]),shellcodes->map[i]);
		}

		printf("\n\n");
*/
		/* prepare for the next one */
		init_shellcode();
	}
	;

identifier
	: namespace SC_COLON SC_COLON SC_ID
	{
		shellcodes->name = strndup(string_get_buffer(), string_get_len());
		string_reset();
	}
	;

namespace
	: SC_XOR
	{
		shellcodes->nspace = sc_xor;
	}
	|
	SC_LINKXOR
	{
		shellcodes->nspace = sc_linkxor;
	}
	|
	SC_KONSTANZXOR
	{
		shellcodes->nspace = sc_konstanzxor;
	}
	|
	SC_LEIMBACHXOR
	{
		shellcodes->nspace = sc_leimbachxor;
	}
	|
	SC_BIND_SHELL
	{
		shellcodes->nspace = sc_bindshell;
	}
	|
	SC_CONNECTBACK_SHELL
	{
		shellcodes->nspace = sc_connectbackshell;
	}
	|
	SC_CONNECTBACK_FILETRANSFER
	{
		 shellcodes->nspace = sc_connectbackfiletransfer;
	}
	|
	SC_EXECUTE
	{
		shellcodes->nspace = sc_execute;
	}
	|
	SC_DOWNLOAD
	{
		shellcodes->nspace = sc_download;
	}
	|
	SC_URL
	{
		shellcodes->nspace = sc_url;
	}
	| 
	SC_BIND_FILETRANSFER
	{
		shellcodes->nspace = sc_bindfiletransfer;
	}
    |
	SC_BASE64
	{
		shellcodes->nspace = sc_base64;
	}
	;

statements
	: /* \epsilon */
	| statement statements
	;

statement
	: inline_statement SC_SEMI
	;

inline_statement
	: pattern
	| flags
	| mapping
	;

flags
	: SC_FLAGS SC_NONE 
	{
		printf("flags none...\n");
	}
	;

mapping
	: SC_MAPPING SC_LPAR map_values SC_RPAR
	;

map_values
	: map_value map_value_comma_list
	;

map_value_comma_list
	: /* \epsilon */
	| SC_COMMA map_value map_value_comma_list
	;

map_value
	: SC_KEY
	{
		shellcodes->map[shellcodes->map_items++] = sc_key;
	}
	| SC_SUBKEY
	{
		shellcodes->map[shellcodes->map_items++] = sc_subkey;
	}
    | SC_SIZE
	{
		shellcodes->map[shellcodes->map_items++] = sc_size;
	}
	| SC_SIZEINVERT
	{
		shellcodes->map[shellcodes->map_items++] = sc_sizeinvert;
	}
	| SC_PORT
	{	
		 shellcodes->map[shellcodes->map_items++] = sc_port;
	}
	| SC_HOST
	{
		shellcodes->map[shellcodes->map_items++] = sc_host;
	}
	| SC_COMMAND
	{
		shellcodes->map[shellcodes->map_items++] = sc_command;
	}
	| SC_URI
   {
	   shellcodes->map[shellcodes->map_items++] = sc_uri;
	}
	| SC_PCRE
   {
	   shellcodes->map[shellcodes->map_items++] = sc_pcre;
	}
	| SC_PRELOAD
   {
	   shellcodes->map[shellcodes->map_items++] = sc_pre;
	}
	| SC_POSTLOAD
   {
	   shellcodes->map[shellcodes->map_items++] = sc_post;
	}
	| SC_NONE
   {
	   shellcodes->map[shellcodes->map_items++] = sc_none;
	}
	| SC_HOSTKEY
   {
	   shellcodes->map[shellcodes->map_items++] = sc_hostkey;
	}
	| SC_PORTKEY
   {
	   shellcodes->map[shellcodes->map_items++] = sc_portkey;
	}

   ;

pattern
	: SC_PATTERN SC_STRING strings
	{
		shellcodes->pattern = strndup(string_get_buffer(), string_get_len());
		shellcodes->pattern_size = string_get_len();
		string_reset();
	}
	;

strings
	:
	| SC_STRING strings
	;

%%

#ifndef HAVE_STRNDUP
 // from http://www.unixpapa.com/incnote/string.html
 char *strndup(const char *str, size_t len)
 {
     char *dup= (char *)malloc( len+1 );
     if (dup) {
         strncpy(dup,str,len);
         dup[len]= '\0';
     }
     return dup;
  }
#endif /* HAVE_STRNDUP */


struct sc_shellcode *init_shellcode()
{
	struct sc_shellcode *s = (struct sc_shellcode *)malloc(sizeof(struct sc_shellcode));

	memset(s, 0, sizeof(struct sc_shellcode));

	s->next = shellcodes;
	shellcodes = s;

	return s;
}


char *sc_get_namespace_by_numeric(int num)
{

	static char *namespacemapping[]=
	{
		"xor",
		"linkxor",
		"konstanzxor",
		"leimbachxor",
		"connectbackshell",
		"connectbackfiletransfer",
		"bindshell",
		"execute",
		"download",
		"url",
		"bindfiletransfer",
        "base64"
	};

	if ( num >= sizeof(namespacemapping)/sizeof(char *) )
		return "unmapped";
	else
		return namespacemapping[num];
}

char *sc_get_mapping_by_numeric(int num)
{
	static char *mapmapping[]=
	{
		"key",
        "subkey",
		"size",
		"sizeinvert",
		"port",
		"host",
		"command",
		"uri",
		"pcre",
		"pre",
		"post",
		"none",
		"hostkey",
		"portkey"
	};
	if ( num >= sizeof(mapmapping)/sizeof(char *) )
		return "unmapped";
	else
		return mapmapping[num];
}



int yyerror(char* s)
{
	snprintf(error_buffer, sizeof(error_buffer),
			 "%s at '%s' on line %d", s, yytext, line_number);
	return 0;
}


int yywrap()
{
	return 1;
}

struct sc_shellcode *sc_parse_file(const char *filename)
{
	yyin = fopen(filename, "r");

	if ( yyin == NULL )
	{
		snprintf(error_buffer, sizeof(error_buffer), "%s", strerror(errno));
		return NULL;
	}

	init_shellcode();
	if ( yyparse() != 0 )
	{
		fclose(yyin);
		/* TODO free partially alloc'd shellcodes */
		return NULL;
	}
	fclose(yyin);
// taken from shellcode-generic/sch_generic_stuttgart.cpp
	return shellcodes;
}

char *sc_get_error()
{
	return error_buffer;
}
