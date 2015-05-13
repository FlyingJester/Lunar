#pragma once

enum LunarTermType {e_lunar_bad_type, e_lunar_span, e_lunar_word};

struct LunarTerm {
    enum LunarTermType type;
    union{
        struct { const char *from, *to, *name; } span;
        struct { const char *term; unsigned len; } word;
    };
    unsigned format_type;
};

struct LunarTree {
    /* Name is usually the program name, prefix is this parse's name */
    const char *name, *prefix;
    const char **breaking_terms;

    const char **format_types;
    unsigned number_format_types;

    struct LunarTerm *terms;
    unsigned number_terms;
};
