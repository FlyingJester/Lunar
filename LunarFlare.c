#include "LunarGen.h"
/* A simple program that generates the Flare editor's parser using Lunar */

static unsigned strlen(const char *a){
    unsigned i = 0;
    while(a[i++]!='\0');
    return i-1;
}

int main(int argc, char *argv[]){

    const char *breaking_terms[29] = {
        " ",
        "\\n",
        "\\r",
        "\\t",
        ">",
        "<",
        "=",
        "-",
        "/",
        "|",
        "\\\\",
        "+",
        "*",
        "&",
        "^",
        "%",
        "!",
        "~",
        ",",
        ".",
        "(",
        ")",
        "{",
        "}",
        "[",
        "]",
        ";",
        ":",
        NULL
    };
    
    const unsigned num_formats = 4, num_keywords = 38;
    const char *format_types[num_formats] = { "comment", "stringliteral", "charliteral", "numberliteral" };
    const char *keywords[38] = {
        "char",
        "short",
        "int",
        "long",
        "unsigned",
        "signed",
        "struct",
        "void",
        "int8_t", 
        "int16_t",
        "int32_t",
        "int64_t",
        "uint8_t",
        "uint16_t",
        "uint32_t",
        "uint64_t",
        "size_t",
        "float",
        "double",
        "if",
        "else",
        "do",
        "while",
        "for",
        "return"
        "switch",
        "continue",
        "break",
        "typedef", 
        "default",
        "case",
        "enum",
        "goto",
        "auto",
        "register",
        "inline",
        "extern",
        "const",
        "static"
    };
    
/*
    const struct LunarTerm terms[] = {};
*/
    const int num_spans = 2;
    struct LunarTerm terms[num_spans+num_keywords] = {
        {e_lunar_span},
        {e_lunar_span}
    };
    
    int i = num_spans;
    
    while(i<num_spans+num_keywords){
        terms[i].type = e_lunar_word;
        terms[i].word.term = keywords[i-num_spans];
        terms[i].word.len = strlen(keywords[i-num_spans]);
        i++;
    }
    
    const struct LunarTree FlareTree = {
        "Flare", "C", 
        breaking_terms,
        format_types,
        num_formats,
        terms,
        num_spans+num_keywords
    };
    
    FILE *flareheader = fopen("FlareParse.h", "wb"),
        *flaresource = fopen("FlareParse.c", "wb");
    if(!(flareheader && flaresource)){
        puts("[LunarFlare] Error could not create files");
        return 1;
    }
    
    /* Build up the terms list */
    terms[0].span.from = "/*";
    terms[0].span.to = "*/";
    terms[0].span.name = "comment1";
    terms[1].span.from = "//";
    terms[1].span.to = "\\n";
    terms[1].span.name = "comment2";
    
    LunarWriteTree(&FlareTree, "FlareParse.h", flareheader, flaresource);

    fclose(flareheader);
    fclose(flaresource);

}
