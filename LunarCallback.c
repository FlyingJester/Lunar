#include "LunarCallback.h"
#include "LunarParse.h"
#include "LunarGen.h"
#include "libfjcsv/csv.h"

#include <string.h>
#include <stdlib.h>

struct LunarParseArg {
    struct LunarTree tree;
    unsigned last_section;
    
    int err;
    char *err_str;
    
};

const char * errs[] = {
    "[Lunar] Warning unknown section ",
    "[Lunar] Empty element ",
    "[Lunar] Invalid element ",
    "[Lunar] Internal error on element ",
};

#define SetError(ERR_N, LEN, IN_STR, OUT_STR)\
do{\
     const unsigned err_len = strlen(errs[ERR_N]);\
     OUT_STR = malloc(err_len + LEN + 1);\
     memcpy(OUT_STR, errs[ERR_N], err_len);\
     memcpy(OUT_STR+err_len, IN_STR, LEN);\
     OUT_STR[err_len+LEN] = 0;\
}while(0)

#define IsSpace(C_) ((C_==' ') || (C_=='\n') || (C_=='\t') || (C_=='\r'))

static unsigned SwallowUntilSpace(const char *in, unsigned i, unsigned len){
    while((i<len) && (!IsSpace(in[i]))) i++;
    return i;
}

static unsigned SwallowSpace(const char *in, unsigned i, unsigned len){
    while((i<len) && IsSpace(in[i])) i++;
    return i;
}

/*
    Section codes:
        'n' - Name section
        's' - Span section
        'S' - Span element
        'w' - Word section
        'W' - Word element
*/

/* A mutually recursive callback and parser? How naughty! */
void LunarParseCallback(const char *str, unsigned long len, int type, void *arg){
    struct LunarParseArg * const parse_arg = arg;
    
    if(parse_arg->err)
        return;
    
    switch(type){
        case e_Bootstrap_keyword:
            if(strncmp(str, "name", len)==0){
                parse_arg->last_section = 'n';
            }
            else if(strncmp(str, "spans", len)==0){
                parse_arg->last_section = 's';
            }
            else if(strncmp(str, "words", len)==0){
                parse_arg->last_section = 'w';
            }
            else{
                SetError(0, len, str, parse_arg->err_str);
                parse_arg->err = 1;
                parse_arg->last_section = 0;
                return;
            }
        break;
        case e_Bootstrap_section:
        {
            if((parse_arg->last_section=='s') || (parse_arg->last_section=='w')){
                parse_arg->last_section+= 'A'-'a';
                LunarParseString(str, len, LunarParseCallback, arg);
            }
            else{
                unsigned n;
                const char **elements = CSV_ParseStringNL(str+1, len-2, &n, ',');
                char **new_elements[3];
                struct LunarTerm new_term;
                if(((parse_arg->last_section=='S') && (n!=3)) ||
                   ((parse_arg->last_section=='W') && (n!=1))){
                    SetError(2, len, str, parse_arg->err_str);
                    parse_arg->err = 1;
                    return;
                }
                
                while(n--){

                    const unsigned start = IsSpace(*(elements[n]))?SwallowSpace(elements[n], 0, strlen(elements[n])):0,
                        end = SwallowUntilSpace(elements[n], start, strlen(elements[n]));
                    new_elements[n] = malloc(end-start+1);
                    new_elements[n][end-start] = 0;
                    memcpy(elements[n]+start, new_elements, end-start);
                    free(elements[n]);
                }
                
                CSV_FreeParse(elements);
                
                if(parse_arg->last_section=='S'){
                    new_term.type = e_lunar_span;
                    new_term.span.from = elements[1];
                    new_term.span.to   = elements[2];
                    new_term.span.name = elements[0];
                }
                else if(parse_arg->last_section=='W'){
                    new_term.type = e_lunar_word;
                    new_term.word.term = elements[0];
                    new_term.word.len = strlen(elements[0]);
                }
                else{
                    SetError(3, len, str, parse_arg->err_str);
                    parse_arg->err = 1;
                }
                
                parse_arg->tree.terms = realloc(parse_arg->tree.terms, ++(parse_arg->tree.number_terms));
                parse_arg->tree.terms[parse_arg->tree.number_terms-1] = new_term;
            }
        }
    }
}
