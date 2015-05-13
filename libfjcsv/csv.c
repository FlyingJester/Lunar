#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

unsigned CSV_CountElements(const char *a, char delimiter){
    unsigned r = 0;

    assert(a);

    while(*a==delimiter) a++;

    if(a[0]!='\0'){
        const char *b = a;
        r=1;
        do{
            if(b[0]==delimiter)
              r++;
        } while(*b++!='\0');

    }
    return r;
}

unsigned CSV_CountElementsN(const char *c, unsigned long n, char delimiter){
    unsigned r = 0;
    const char *a = c;
    assert(a);
    
    while(*a==delimiter) a++;
    
    if(a-c<n){
        const char *b = a;
        r=1;
        do{
            if(b[0]==delimiter)
              r++;
        } while((b++)-c<n);

    }
    return r;
    
}

const char **CSV_ParseStringNL(const char *a, unsigned long l, unsigned long *n, char delimiter){
    int i, elements = CSV_CountElements(a, delimiter);
    char **r = malloc(sizeof(const char*)*(elements+1));
    const char *b;

    if(n)
        n[0] = elements;

    while(*a==delimiter) a++;
    b=a;

    assert(a);
    assert(r);

    for(i = 0; i<elements; i++){
        const char *c = memchr(b, delimiter, l-(b-a));

        if(c==NULL){
			
			const unsigned b_len = l-(b-a);
			
            assert(i==elements-1);
			
			r[i] = malloc(b_len+1);
			r[i][b_len] = '\0';
			memcpy(r[i], b, b_len);

            assert(r[i]);
        }
        else{
            /* Minus one to push back before the comma.
            */
            unsigned len = (c-b);
            assert(c>b);

            r[i] = malloc(len+1);

            assert(r[i]);

            r[i][len]='\0';

            memcpy(r[i], b, len);

        }
        /* Plus one to outrun the comma.
        */
        b = c+1;
    }

    r[i] = NULL;

    return (const char **)r;
}

const char ** CSV_ParseStringN(const char *a, unsigned long b, char c){
    return CSV_ParseStringNL(a, b, NULL, c);
}

const char **CSV_ParseString(const char *a, char delimiter){
    return CSV_ParseStringN(a, strlen(a), delimiter);
}

const char *CSV_ConstructString(const char **a, char delimiter){

    char *r = NULL, *c = NULL;
    int i = 0, len = 0;
    assert(a);

    {
        const char *b = a[i++];

        /* the `len+=1' provides room for the comma, except on the last iteration.
         Then, it provides room for the NULL.
        */
        while(b!=NULL){
            printf("Parse 1: %s\n", b);
            len+=strlen(b)+1;
            b = a[i++];
        }

    }

    if(len==0){
		char *ret_str = malloc(1);
		ret_str[0] = '\0';
		return ret_str;
	}
	
    printf("Parse n: %i\n", len);
    r = malloc(len);
    r[len-1] = '\0';

    c = r;

    i = 0;

    if(len){
        const char *b = a[i++];
        while(b!=NULL){
            int olen = strlen(b);

            memcpy(c, b, olen);

            printf("Parse 2: %s\n", c);

            c[olen] = delimiter;

            c+=olen+1;


            b = a[i++];

            assert(c<=r+len);

        }
    }
    r[len-1] = '\0';

    return r;

}

void CSV_FreeParse(const char **a){

    assert(a);

    {
        const char *b = a[0];
        int i = 0;

        while(b!=NULL){

            assert(b);

            free((void *)b);
            b = a[++i];
        }
    }

    free(a);

}
