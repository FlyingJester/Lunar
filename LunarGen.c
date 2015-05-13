#include "Lunar.h"
#include "LunarGen.h"
#include <stdlib.h>

static unsigned strlen(const char *a){
    unsigned i = 0;
    while(a[i]!='\0') i++;
    return i;
}

static const char *s_l_pragma = "#pragma ";
static const char *s_l_include = "#include ";

static void write_line(const char *s, FILE *f){
    fputs(s, f);
    fputc('\n', f);
}

static void write_2s_line(const char *a, const char *b, FILE *f){
    fputs(a, f);
    fputs(b, f);
    fputc('\n', f);
}

static void write_pragma(const char *p, FILE *f){ write_2s_line(s_l_pragma, p, f); }

enum header_type {e_h_literal = 0, e_h_system, e_h_local};

static void write_include(const char *h, enum header_type i, FILE *f){
    fputs(s_l_include, f);
    if(i==e_h_system){
        fputc('<', f);
        fputs(h, f);
        fputc('>', f);
    }
    else if(i==e_h_local){
        fputc('"', f);
        fputs(h, f);
        fputc('"', f);
    }
    fputc('\n', f);
}

static void write_ifdef(const char *d, FILE *f){ write_2s_line("#ifdef ", d, f); }
static void write_endif(FILE *f){ write_line("#endif", f); }

struct decl_arg{
    const char * const type, * const name;
    unsigned char indirection;
};

static void write_decl(const char *type, const char *prefix, const char *name, const struct decl_arg *args, unsigned number_args, int is_decl, FILE *f){

    int i = 0;
    fputs(type, f);
    fputc(' ', f);
    fputs(prefix, f);
    fputs(name, f);
    fputc('(', f);
    while(i<number_args){
        fputs(args[i].type, f);
        if(args[i].indirection){
            int e = 0;
            fputc(' ', f);
            while(e++<args[i].indirection)
                fputc('*', f);
        }
        
        if(args[i].name[0]!='\0'){
            fputc(' ', f);
            fputs(args[i].name, f);
        }
        
        if(++i<number_args)
            fputs(", ", f);
    }
    
    fputc(')', f);
    
    if(is_decl)
        fputc(';', f);
    else
        fputc('{', f);
    
    fputc('\n', f);
} 

void LunarWriteTree(const struct LunarTree *tree, const char *header_name, FILE *header, FILE *source){
    /* Enumerate all the term types to header */
    write_pragma("once", header);
  /*  write_include("stdlib.h", e_h_system, header);
    */
    { /* Write the enum for the terms. */
        int i=0;
        fprintf(header, "enum %sFormat_%s {", tree->name, tree->prefix);
        
        #define WRITE_ENUM_TERM(W) fprintf(header, " e_%s_%s", tree->prefix, W)
        
        WRITE_ENUM_TERM("fail,");
        WRITE_ENUM_TERM("normal,");
        WRITE_ENUM_TERM("keyword,");
        
        while(i<tree->number_format_types){
            WRITE_ENUM_TERM(tree->format_types[i++]);
            fputc(',', header);
        }
        
        WRITE_ENUM_TERM("n_formats");
        
        write_line(" };", header);
    }
    
    fputc('\n', header);
    
    { /* Write the parser definition */
        const unsigned number_parse_args = 4;
        const struct decl_arg args[number_parse_args] = {
            {"const char", "string", 1},
            {"unsigned long", "len"},
            {"void", "(*parse_callback)(const char *, unsigned long, int, void *)"},
            {"void", "arg", 1}
        };
        
        write_ifdef("__cplusplus", header);
        write_line("extern \"C\"", header);
        write_endif(header);
        write_decl("int", tree->name, "ParseString", args, number_parse_args, 1, header);
    }
    fputc('\n', header);
    fflush(header);
    
    /* Generate Code */
    write_include(header_name, e_h_local, source);
    write_include("string.h", e_h_system, source);
    write_include("assert.h", e_h_system, source);
    write_include("ctype.h", e_h_system, source);
    
    fputc('\n', source);
    /* Generate span swallowers */
    {
        const struct decl_arg args[2] = {
            {"const char", "string", 2},
            {"unsigned long", "len", 1}
        };
        int i = 0;
        for(; i<tree->number_terms; i++){
            if(tree->terms[i].type!=e_lunar_span)
                continue;
            {
                const unsigned to_len = strlen(tree->terms[i].span.to);
                                
                write_decl("static int", "ParseString_", tree->terms[i].span.name, args, 2, 0, source);
                fprintf(source, "    while((**string!='\\0') && (*len>%i)){\n", to_len);
                write_line("        if(**string=='\\\\'){", source);
                write_line("            string[0]++;", source);
                write_line("            len[0]--;", source);
                write_line("        }", source);
                /* For single characters/bytes, don't bother with a memcmp. */
                if((to_len==1) || ((to_len==2) && (*(tree->terms[i].span.to)=='\\'))){
                    if((tree->terms[i].span.to[0]=='\'') && (tree->terms[i].span.to[1]=='\0'))
                        write_line("        else if(**string=='\\''){", source);
                    else
                        fprintf(source, "        else if(**string=='%s'){\n", tree->terms[i].span.to);
                }
                else{
                    fprintf(source, "        else if(memcmp(*string, \"%s\", %i)==0){\n", tree->terms[i].span.to, to_len);
                }
            }
            write_line("            string[0]++;", source);
            write_line("            len[0]--;", source);
            write_line("            return 1;", source);
            write_line("        }", source);
            write_line("        string[0]++;", source);
            write_line("        len[0]--;", source);
            write_line("   }", source);
            write_line("   return 1;", source);
            write_line("}", source);
            fputc('\n', source);
        }
    }
   
    /* Generate breaking char finder */
    {
        const struct decl_arg arg = {"unsigned", "c"};
        char n[32];
        int i = 0;
        write_decl("static int", "CharIsBreaking", "", &arg, 1, 0, source);
        
        /* Write all one-char breakers */
        write_line("    switch(c){", source);
        while(tree->breaking_terms[i]!=NULL){
            if(tree->breaking_terms[i][0]=='\0')
                continue;
        
            if((tree->breaking_terms[i][1]=='\0') ||
              ((tree->breaking_terms[i][0]=='\\') && (tree->breaking_terms[i][2]=='\0'))){
                fprintf(source, "        case '%s':\n", tree->breaking_terms[i]);
            }
            i++;
        }
        
        write_line("        return 1;", source);
        write_line("    }", source);
        
        
        write_line("    return 0;", source);
        write_line("}", source);
    }
    
    fputc('\n', source);
    
    /* Write the surrounded-by-spaces function */
    {
        const unsigned argc = 5;
        const struct decl_arg args[argc] = {
            {"const char", "buffer", 1},
            {"const char", "const constant", 1},
            {"unsigned long", "constant_byte_len"}, 
            {"unsigned long", "len"}, 
            {"unsigned long", "first_to_constant_end", 1}};
        write_decl("static int", "TermSurroundedBySpaces", "", args, argc, 0, source);
        write_line("    /*const char * const end = buffer+len;*/", source);
        write_line("    if(len==0)", source);
        write_line("        return 0;", source);
        write_line("    else {", source);
        write_line("        const unsigned first_len = 1, /*last_len = 1, */first = *buffer; /* TODO: UTF */", source);
        write_line("        if(first_len+constant_byte_len>len)", source);
        write_line("            return 0;", source);
        write_line("        else{", source);
        write_line("            const unsigned last = buffer[first_len+constant_byte_len];", source);
        write_line("            first_to_constant_end[0] = first_len+constant_byte_len;", source);
        write_line("            return CharIsBreaking(first) && ", source);
        write_line("                CharIsBreaking(last) && ", source);
        write_line("                (memcmp(buffer+first_len, constant, constant_byte_len)==0);", source);
        write_line("        }", source);
        write_line("    }", source);
        write_line("    return 0; /* This is unreachable. Some compilers don't thinks so, though. */", source);
        write_line("}", source);
    }
    
    {
    /* Generate main parse function. */
        unsigned i = 0;
        const unsigned argc = 4;
        const struct decl_arg args[argc] = {
            {"const char", "string", 1},
            {"unsigned long", "len"},
            {"void", "(*parse_callback_)(const char *, unsigned long, int, void *)"},
            {"void", "arg_", 1}};
        write_decl("int", tree->name, "ParseString", args, argc, 0, source);
        write_line("    const char *old_start = string, *at = string;", source);
        write_line("    unsigned long skipped;", source);
        write_line("    while((*at!='\\0') && (len>0)){", source);
        write_line("        skipped = 0;", source);
        write_line("        if(0);", source);
        /* Write all spans */
        for(; i<tree->number_terms; i++){
            if(tree->terms[i].type==e_lunar_span){
                unsigned e = 0;
                const unsigned from_len = strlen(tree->terms[i].span.from);
                unsigned real_len = from_len;
                
                /* We check one ahead of from_len in the case that the last char is a trailing \. 
                    But that's OK, we'll just hit the NULL.
                 */
                while(e<from_len){
                    if(tree->terms[i].span.from[e++]=='\\'){
                        if(tree->terms[i].span.from[e]=='\\') e++;
                        real_len++;
                    }
                }
                
                if(real_len==1){
                    /* We are changine quote types... */
                    if((tree->terms[i].span.from[0]=='\'') && (tree->terms[i].span.from[1]=='\0')){
                        write_line("        else if(*string=='\\''){", source);
                    }
                    else{
                        fprintf(source, "        else if(*string==\'%s\'){\n", tree->terms[i].span.from);
                    }
                    write_line("            at++;\n", source);
                }
                else{ /* TODO: Unroll the memcmp. */
                    fprintf(source, "        else if((len>%i) && (memcmp(string, \"%s\", %i)==0)){\n", real_len, tree->terms[i].span.from, real_len);
                    fprintf(source, "            at+=%i;\n", real_len);
                }
                fprintf(source, "            if(!ParseString_%s(&at, &len))\n", tree->terms[i].span.name);
                write_line("                return 0;", source);
                fprintf(source, "            parse_callback_(string, at-string,  e_%s_%s, arg_);", tree->prefix, tree->format_types[tree->terms[i].format_type]);
                write_line("        }", source);
            }
        }
        
        write_line("        else if(0 ||", source);
        
        for(i=0; i<tree->number_terms; i++){
            if(tree->terms[i].type==e_lunar_word){
                fprintf(source, "            TermSurroundedBySpaces(string, \"%s\", %i, len, &skipped) ||\n", tree->terms[i].word.term, tree->terms[i].word.len);
            }
        }
        write_line("            0){", source);
        write_line("            at+=skipped;", source);
        
        /* Should be string+utflen(*string) */
        fprintf(source, "            parse_callback_(string+1, skipped-1, e_%s_keyword, arg_);\n", tree->prefix);
        write_line("        }", source);
        write_line("        else{", source);
        write_line("            len--;", source);
        write_line("            string = ++at;", source);
        write_line("            continue;", source);
        write_line("        }", source);
        write_line("        assert(at>string);", source);
        write_line("        if(string!=old_start)", source);
        fprintf(source, "            parse_callback_(old_start, string-old_start, e_%s_normal, arg_);\n", tree->prefix);
        write_line("         if(!((at[0]!='\\0') && len))", source);
        write_line("             return 1;", source);
        write_line("         if(at-string>=len)", source);
        write_line("             break;", source);
        write_line("         else", source);
        write_line("             len-=at-string;", source);
        write_line("         old_start = string = at;", source);
        write_line("     }", source);
        fputc('\n', source);
        write_line("     if(string-old_start>0)", source);
        fprintf(source, "         parse_callback_(old_start, string-old_start, e_%s_normal, arg_);\n", tree->prefix);
        write_line("     return 1;", source);
        write_line("}", source);
    }
    
    fputc('\n', source);
    fflush(source);
    
}
