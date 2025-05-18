#ifndef ARRAYLIST_H
#define ARRAYLIST_H
/*

Define variable with
T* arr = NULL;

access the data with
arr[i]

                    IMPORTANT!
if the array is expanded the pointer will be invalidated
if you pass an arr as an input to a function you must pass it by reference ie T** 
or you must return the updated pointer when you are done

functions with can invalidate the pointer include
arrlist_setcap
arrlist_setcount
arrlist_expand
arrlist_append
arrlist_insert
arrlist_insertn
arrlist_destroy
all other functions will not invalidate the pointer

function signitures (actually macros) all start with arrlist_
the underlying functions are not obfuscated and are reachable by adding _func to the end of the function, although not all have a corresponding function
the underlying functions are not erganomic to use and as such the macros are given
*/

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

// header for the automatically expanding buffer
typedef struct ArrayListHeader {
    uint32_t capacity;  // capacity of array
    uint32_t count;       // length of array
    uint8_t  data[];     // beggining of buffer (decays to pointer)
} ArrayListHeader;

#define ARRAYLIST_INITIAL_CAPACITY 8 // size it allocates if you expand a zero capacity array

#define arrlist_header(arr)     ((ArrayListHeader*)(arr) - 1)                           // ArrayListHeader* arrlist_header(arr) returns the pointer to the header of the array, doesn't check if pointer is NULL
#define arrlist_count(arr)        ((arr) ? ((ArrayListHeader*)(arr) - 1)->count : 0)        // uint32_t arrlist_count(arr)  returns the len (zero if arr is NULL), no l value if you want to increment len manually use arrlist_header(arr)->count++
#define arrlist_cap(arr)        ((arr) ? ((ArrayListHeader*)(arr) - 1)->capacity : 0)   // uint32_t arrlist_capacity(arr)  returns the capacity (zero if arr is NULL), no l value you can manually change the capacity with arrlist_header(arr)acity, although you probably shouldn't

void* arrlist_make_f(uint32_t n, uint32_t element_size);
void* arrlist_setcap_f(void* arr, uint32_t n, uint32_t element_size);
void* arrlist_expand_f(void* arr, uint32_t n, uint32_t element_size);

void* arrlist_insertn_f(void* restrict arr, uint32_t i, const void* restrict buffer, uint32_t n, uint32_t element_size);
void* arrlist_removen_f(void* arr, uint32_t i, uint32_t n, uint32_t element_size);

#define arrlist_make_lvalue(temp) (typeof((*arr))[1]){(temp)}
#define arrlist_bounds_check(arr, i) assert((i) >= 0 && (i) < arrlist_count(arr) && "accessing out of bounds")
#define arrlist_bounds_check_inclusive(arr, i) assert((i) > 0 && (i) <= arrlist_count(arr) && "accessing out of bounds")

#define arrlist_front(arr) (arr)[(assert(arrlist_count(arr) > 0), 0)] // T arrlist_front(T* arr) returns the first element
#define arrlist_back(arr) (arr)[(assert(arrlist_count(arr) > 0), arrlist_count(arr) - 1)] // T arrlist_back(T* arr) returns the last element in the list

#define arrlist_free(arr)                   ((void)((arr) ? free(arrlist_header(arr)) : (void)0), (arr) = NULL)                 // T* arrlist_destroy(T* arr)  frees the memory of the array and sets arr to NULL, returns NULL
#define arrlist_expand(arr, n)              (arrlist_count(arr) + n > arrlist_cap(arr) ? (void)((arr) = arrlist_expand_f(arr, n, sizeof(*arr))) : (void)0)                                    // void arrlist_expand(T* arr, uint32_t n)  expands the arrlist so that it can fit at least n elements, but only if len + n > capacity
#define arrlist_setcap(arr, n)              ((arr) = arrlist_setcap_f(arr, n, sizeof(*arr)))                                    // void arrlist_setcap(T* arr, uint32_t n)  sets the capacity to n, does nothing if capacity >= n
#define arrlist_setcount(arr, n)              ((arr) = arrlist_setcap_f(arr, n, sizeof(*arr)), arrlist_header(arr)->count = (n))    // int arrlist_setcount(T* arr, uint32_t n)  sets the len to n, expands as needed, returns the new len

#define arrlist_append(arr, element)        ((arr) = arrlist_expand_f(arr, 1, sizeof(*arr)), (arr)[arrlist_header(arr)->count++] = (element))                             // T arrlist_append(T* arr, T element)  appends element to the end of the arrlist and returns the element pushed
#define arrlist_pop(arr)                    (assert(arrlist_count(arr) > 0 && "attempting to pop from empty list"), arrlist_header(arr)->count--, (arr)[arrlist_count(arr)])  // T arrlist_pop(T* arr)  pops an element from the end of the list and returns it (cast to void to avoid unused value warnings or use arrlist_header(arr)->count-- instead)

#define arrlist_insert(arr, element, i)     (arrlist_bounds_check_inclusive(arr, i), (arr) = arrlist_insertn_f(arr, i, &arrlist_make_lvalue(element), 1, sizeof(*arr)))    // T arrlist_insert(T* arr, T element, uint32_t i) inserts element so that it is at the ith index of arr, moves elements over as needed, returns the element pushed
#define arrlist_insertn(arr, buffer, n, i)  (arrlist_bounds_check_inclusive(arr, i), (arr) = arrlist_insertn_f(arr, i, buffer, n, sizeof(*arr)))                           // void arrlist_insertn(T* arr, T buffer, uint32_t n, uint32_t i) inserts n elements starting at the ith index, moves elements over as needed

#define arrlist_remove(arr, i)              (arrlist_bounds_check(arr, i), (arr) = arrlist_removen_f(arr, i, 1, sizeof(*arr)))    // void arrlist_remove(T* arr, uint32_t i) removes element from arr at index i, keeps relative orderings, returns the element removed
#define arrlist_removen(arr, n, i)          (arrlist_bounds_check(arr, i), (arr) = arrlist_removen_f(arr, i, n, sizeof(*arr)))    // void arrlist_removen(T* arr, uint32_t n, uint32_t i) removes n elements from arr starting at index i, keeps relative orderings
#define arrlist_remswap(arr, i)             (arrlist_bounds_check(arr, i), (arr[i] = arr[--arrlist_header(arr)->count]))            // T arrlist_remswap(T* arr, uint32_t i) removes the ith element replacing it with the last element, returns element which replaced it

#define arrlist_swap(arr, i, j)             do {arrlist_bounds_check(arr, i); arrlist_bounds_check(arr, j); typeof(*arr) _temp = (arr)[i]; (arr)[i] = (arr)[j]; (arr)[j] = _temp;} while(0) // void arrlist_swap(T* arr, uint32_t i, uint32_t j) swaps the elements at index i and index j

#define arrlist_clone(arr)                  arrlist_insertn_func(NULL, 0, arr, arrlist_count(n), sizeof(*arr)) // T* arrlist_clone(T* arr) clones the arrlist, returns the clone

#define arrlist_foreach(item, arr)          for (typeof(arr) item = (arr); item != (arr) + arrlist_count(arr); ++item)
#define arrlist_print(arr, fmt, del) do{for(size_t i = 0; i < arrlist_count(arr); i++) {printf(fmt, (arr)[i]); printf(del);} putc('\n', stdout);} while (0);
#ifdef  ARRAYLIST_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

void* arrlist_make_f(uint32_t n, uint32_t element_size) {
    ArrayListHeader* header = malloc(sizeof(ArrayListHeader) + n * element_size);
    assert(header && "malloc failed loser!");
    header->count = 0;
    header->capacity = n;
    return header + 1;
}

void* arrlist_setcap_f(void* arr, uint32_t n, uint32_t element_size) {
    if (!arr) {
        return arrlist_make_f(n, element_size);
    }
    ArrayListHeader* header = arrlist_header(arr);

    header = realloc(header, sizeof(ArrayListHeader) + n * element_size);
    assert(header != NULL && "realloc failed");
    header->capacity = n;
    if (header->count > n) header->count = n;
    return header + 1;
}
void* arrlist_expand_f(void* arr, uint32_t n, uint32_t element_size) {
    if (!arr) {
        uint32_t capacity = n > ARRAYLIST_INITIAL_CAPACITY ? n : ARRAYLIST_INITIAL_CAPACITY;
        return arrlist_make_f(capacity, element_size);
    }
    ArrayListHeader* header = arrlist_header(arr);

    if (header->count + n <= header->capacity) return arr;

    if (header->capacity * 2 > header->count + n) {
        header->capacity *= 2;
    } else {
        header->capacity = header->count + n;
    }
    header = realloc(header, sizeof(ArrayListHeader) + header->capacity * element_size);
    assert(header != NULL && "realloc failed");
    return header->data;
}
void* arrlist_insertn_f(void* restrict arr, uint32_t i, const void* restrict buffer, uint32_t n, uint32_t element_size) {
    assert(i <= arrlist_count(arr) && "attempting to insert out of bounds");
    arr = arrlist_expand_f(arr, n, element_size); 
    memmove((char*)arr + (i + n) * element_size, (char*)arr + i * element_size, (arrlist_count(arr) - (i)) * element_size); 
    memcpy((char*)arr + i * element_size, buffer, n * element_size); 
    arrlist_header(arr)->count += n;
    return arr;
}
void* arrlist_removen_f(void* arr, uint32_t i, uint32_t n, uint32_t element_size) {
    assert((i) <= arrlist_count(arr) && "attempting to remove out of bounds");
    memmove((char*)arr + i * element_size, (char*)arr + (i+n)*element_size, (arrlist_count(arr) - (i)) * element_size); 
    arrlist_header(arr)->count-=n;
    return arr;
}
#endif // #ifdef ARRAYLIST_IMPLEMENTATION
#endif // #ifndef ARRAYLIST_H