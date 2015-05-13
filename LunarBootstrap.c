#include "LunarGen.h"
#include <stdio.h>
/* Constructs a parse tree that will generate Lunar's input parser. */

int main(int argc, char *argv[]){
    const int breaking_termsc = 6;
    const char *breaking_terms[breaking_termsc+1] = {
        " ",
        "\\n",
        "\\r",
        "\\t",
        "{",
        "}",
        NULL
    };
    
    const char *format_types[1] = { "section" };
    
    struct LunarTerm terms[4] = {
        {e_lunar_span}, /* Section Spans */
        {e_lunar_word}, /* Name Section */
        {e_lunar_word}, /* Span Section */
        {e_lunar_word}, /* Word Section */
    };
    
    const struct LunarTree BootstrapTree = {
        "Lunar", "Bootstrap", 
        breaking_terms,
        format_types,
        1,
        terms,
        4
    };
    
    FILE *bootstrapheader = fopen("LunarParse.h", "wb"),
        *bootstrapsource = fopen("LunarParse.c", "wb");
    if(!(bootstrapheader && bootstrapsource)){
        puts("[LunarBootstrap] Error could not create files");
        return 1;
    }
    
    /* Build up the terms list */
    terms[0].span.from = "{";
    terms[0].span.to = "}";
    terms[0].span.name = "section";
    terms[1].word.term = "name";
    terms[1].word.len  = 4;
    terms[2].word.term  = "spans";
    terms[2].word.len  = 5;
    terms[3].word.term  = "words";
    terms[3].word.len  = 5;
    
    LunarWriteTree(&BootstrapTree, "LunarParse.h", bootstrapheader, bootstrapsource);

    fclose(bootstrapheader);
    fclose(bootstrapsource);
    
}
