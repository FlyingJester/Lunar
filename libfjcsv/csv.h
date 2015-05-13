#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Individual strings returned should be `free'ed.
 Parse parameters (const char **) should have a NULL element on the end.
*/

unsigned CSV_CountElements(const char *, char);
unsigned CSV_CountElementsN(const char *, unsigned long, char);
const char **CSV_ParseString(const char *, char);
const char **CSV_ParseStringNL(const char *, unsigned long, unsigned *, char);
const char **CSV_ParseStringN(const char *, unsigned long, char);
const char *CSV_ConstructString(const char **, char);
void CSV_FreeParse(const char **);

#ifdef __cplusplus
}

#include <string>
#include <vector>
#include <algorithm>

namespace FJ {

    namespace CSV {

        inline unsigned CountElements(const char *a, char delimiter = ','){
            return CSV_CountElements(a, delimiter);
        }

        template<typename T>
        unsigned CountElements(const T &a, char delimiter = ','){
            return CSV_CountElements(a.c_str(), delimiter);
        }

        inline const char **ParseString(const char *a, char delimiter = ','){
            return CSV_ParseString(a, delimiter);
        }

        inline const char **ParseString(char *const a, char delimiter = ','){
            return CSV_ParseString(a, delimiter);
        }

        template<typename T>
        const char **ParseString(const T &a, char delimiter = ','){
            return CSV_ParseString(a.c_str(), delimiter);
        }

        template<class T>
        const char *ConstructString(T &container, char delimiter = ','){

            std::vector<typename T::T> vec(container.begin(), container.end());

            return CSV_ConstructString(&(vec.front()), delimiter);
        }

        template<>
        inline const char *ConstructString<const char **>(const char ** &c_array, char delimiter){
            return CSV_ConstructString(c_array, delimiter);
        }

        template<>
        inline const char *ConstructString<std::vector<const char *> >(std::vector<const char *> &container, char delimiter){
            return CSV_ConstructString(&(container.front()), delimiter);
        }


        template<class T>
        class FunctionalStringConverter {
        public:
            const char *operator () (const T &str){
                return str.c_str();
            }
        };

        template<>
        inline const char *ConstructString<std::vector<std::string> >(std::vector<std::string> &container, char delimiter){

            std::vector<const char *> vec(container.size());
            std::for_each(container.begin(), container.end(), FunctionalStringConverter<std::string>());

            return CSV_ConstructString(&(vec.front()), delimiter);
        }


        inline const char *ConstructString(const char ** c_array, char delimiter = ' '){
            return CSV_ConstructString(c_array, delimiter);
        }

        inline void FreeParse(const char **a){
            CSV_FreeParse(a);
        }

    }

}

#endif
