#ifndef GAPBUFFER_H_
#define GAPBUFFER_H_

#include "stringbuilder.h"

#include "short_types.h"

typedef struct GapBuffer {
    char* data;
    isize gap_begin;
    isize gap_end;
    isize capacity;
} GapBuffer;

typedef struct GapBufSlice {
    String l;
    String r;
} GapBufSlice;

isize gapbuf_gaplen(GapBuffer* gapbuf);
isize gapbuf_count(GapBuffer* gapbuf);

GapBuffer gapbuf_with_cap(isize cap);
void gapbuf_free(GapBuffer* gapbuf);

GapBufSlice gapbuf_getstrings(GapBuffer* gapbuf);
GapBufSlice gapbuf_slice(GapBuffer* gapbuf, isize start, isize end);

void gapbuf_expand(GapBuffer* gapbuf, isize n);

void gapbuf_movegap_rel(GapBuffer* gapbuf, isize n);
void gapbuf_movegap(GapBuffer* gapbuf, isize n);

void gapbuf_clear(GapBuffer* gapbuf);

void gapbuf_insert(GapBuffer* gapbuf, char c);
void gapbuf_insertn(GapBuffer* gapbuf, const char* buf, isize n);

void gapbuf_remove(GapBuffer* gapbuf);
String gapbuf_removen(GapBuffer* gapbuf, isize n);

void gapbuf_remove_after(GapBuffer* gapbuf);
String gapbuf_removen_after(GapBuffer* gapbuf, isize n);

void gapbuf_print(GapBuffer* gapbuf);

void gapbuf_read_entire_file(GapBuffer* gapbuf, const char* filename);
void gapbuf_write_entire_file(GapBuffer* gapbuf, const char* filename);

#ifdef GAPBUFFER_IMPLEMENTATION
isize gapbuf_gaplen(GapBuffer* gapbuf) {
    return gapbuf->gap_end - gapbuf->gap_begin;
}
isize gapbuf_count(GapBuffer* gapbuf) {
    return gapbuf->capacity - gapbuf_gaplen(gapbuf);
}
GapBuffer gapbuf_with_cap(isize cap) {
    GapBuffer gapbuf = {
        .data = malloc(cap),
        .capacity = cap,
        .gap_begin = 0,
        .gap_end = cap,
    };
    return gapbuf;
}
void gapbuf_free(GapBuffer* gapbuf) {
    free(gapbuf->data);
    *gapbuf = (GapBuffer){0};
}

isize gapbuf_rawidx(GapBuffer* gapbuf, isize index) {
    assert(index >= 0 && index < gapbuf_count(gapbuf));
    return index < gapbuf->gap_begin ? index : index + gapbuf->gap_end;
}
char gapbuf_get(GapBuffer* gapbuf, isize index) {
    return gapbuf->data[gapbuf_rawidx(gapbuf, index)];
}

GapBufSlice gapbuf_getstrings(GapBuffer* gapbuf) {
    GapBufSlice slice = {
        .l = {.data = gapbuf->data, .count = gapbuf->gap_begin},
        .r = {.data = gapbuf->data + gapbuf->gap_end, .count = gapbuf->capacity - gapbuf->gap_end},
    };
    return slice;
}
GapBufSlice gapbuf_slice(GapBuffer* gapbuf, isize start, isize end) {
    if (start >= gapbuf->gap_begin) {
        printf("1\n");
        // start is after gap
        // "abcdef[   ]ghihjlk"
        //             ^     ^
        //             s     e
        start += gapbuf_gaplen(gapbuf);
        end += gapbuf_gaplen(gapbuf);

        GapBufSlice slice = {
            .l = {.data = gapbuf->data + start, .count = end - start},
            .r = {.data = NULL, .count = 0},
        };
        return slice;
    } else if (end < gapbuf->gap_begin) {
        printf("2\n");
        // end is before gap
        // "abcdef[   ]ghihjlk"
        //  ^   ^
        //  s   e
        GapBufSlice slice = {
            .l = {.data = gapbuf->data + start, .count = end - start},
            .r = {.data = NULL, .count = 0},
        };
        return slice;
    } else {
        printf("3\n");
        // start and end are on opposite sides of the gap
        // "abcdef[   ]ghihjlk"
        //  ^               ^
        //  s               e
        GapBufSlice slice = {
            .l = {.data = gapbuf->data + start, .count = gapbuf->gap_begin - start},
            .r = {.data = gapbuf->data + gapbuf->gap_end + end, .count = end - gapbuf->gap_end},
        };
        return slice;
    }
}

void gapbuf_expand(GapBuffer* gapbuf, isize n) {
    isize new_capacity = gapbuf->capacity * 2;
    if (n > new_capacity) new_capacity = n;

    char* new_buffer = malloc(new_capacity);
    assert(new_buffer && "malloc failed");

    GapBufSlice strings = gapbuf_getstrings(gapbuf);
    isize begin = strings.l.count;
    isize end   = new_capacity - strings.r.count;
    // copies left to the beggining of the buffer and then creates a gap after the end of left and puts right at the end
    //  <----------> l.count     <---------> r.count
    // "llllllllllll[           ]rrrrrrrrrrr"
    //  ^ new_buffer             ^ new_buffer + new_capacity - strings.r.count
    memcpy(new_buffer, strings.l.data, strings.l.count);
    memcpy(new_buffer + end, strings.r.data, strings.r.count);

    free(gapbuf->data);

    gapbuf->gap_begin = begin;
    gapbuf->gap_end = end;
    gapbuf->capacity = new_capacity;
    gapbuf->data = new_buffer;
}
void gapbuf_movegap_rel(GapBuffer* gapbuf, isize n) {
    if (gapbuf->gap_end + n > gapbuf->capacity || gapbuf->gap_begin + n < 0) {
        assert(0 && "attempting to move gap out of bounds");
    }
    if (n == 0) return;

    char* gap_begin_p = gapbuf->data + gapbuf->gap_begin;
    char* gap_end_p = gapbuf->data + gapbuf->gap_end;

    if (n > 0) {
        memcpy(gap_begin_p, gap_end_p, labs(n));
    } else {
        memcpy(gap_end_p + n, gap_begin_p + n, labs(n));
    }

    gapbuf->gap_begin += n;
    gapbuf->gap_end += n;
}
void gapbuf_movegap(GapBuffer* gapbuf, isize index) {
    assert(index >= 0 && index < gapbuf_count(gapbuf) && "index out of bounds");

    isize n = index - gapbuf->gap_begin;
    gapbuf_movegap_rel(gapbuf, n);
}

void gapbuf_clear(GapBuffer* gapbuf) {
    gapbuf->gap_begin = 0;
    gapbuf->gap_end = gapbuf->capacity;
}

void gapbuf_insert(GapBuffer* gapbuf, char c) {
    if (gapbuf_gaplen(gapbuf) == 0) gapbuf_expand(gapbuf, 1);

    gapbuf->data[gapbuf->gap_begin++] = c;
}
void gapbuf_insertn(GapBuffer* gapbuf, const char* buf, isize n) {
    if (gapbuf_gaplen(gapbuf) < n) gapbuf_expand(gapbuf, n);

    memcpy(gapbuf->data + gapbuf->gap_begin, buf, n);
    gapbuf->gap_begin += n;
}
void gapbuf_remove(GapBuffer* gapbuf) {
    if (gapbuf->gap_begin == 0) return;
    gapbuf->gap_begin--;
}
String gapbuf_removen(GapBuffer* gapbuf, isize n) {
    isize len = 0;
    if (gapbuf->gap_begin <= n) {
        len = gapbuf->gap_begin;
        gapbuf->gap_begin = 0;
    } else {
        gapbuf->gap_begin -= n;
        len = n;
    }
    return (String) {.data = gapbuf->data + gapbuf->gap_begin, .count = len};
}

void gapbuf_remove_after(GapBuffer* gapbuf) {
    if (gapbuf->gap_end == gapbuf->capacity) return;
    gapbuf->gap_end++;
}

String gapbuf_removen_after(GapBuffer* gapbuf, isize n) {
    isize len = 0;
    if (gapbuf->gap_end + n > gapbuf->capacity) {
        gapbuf->gap_end = gapbuf->capacity;
        len = gapbuf->capacity - gapbuf->gap_end;
    } else {
        gapbuf->gap_end += n;
        len = n;
    }
    return (String) {.data = gapbuf->data + gapbuf->gap_end - len, .count = len};

}

void gapbuf_print(GapBuffer* gapbuf) {
    GapBufSlice strings = gapbuf_getstrings(gapbuf);
    string_print(strings.l);
    string_print(strings.r);
}

void gapbuf_debug(GapBuffer* gapbuf) {
    for (isize i = 0; i < gapbuf->capacity; i++) {
        if (i < gapbuf->gap_begin || i >= gapbuf->gap_end) {
            if (gapbuf->data[i] == '\n' || gapbuf->data[i] == '\r') {
                putc('/', stdout);
            } else {
                putc(gapbuf->data[i], stdout);
            }
        } else {
            putc('-', stdout);
        }
    }
    putc('-', stdout);
    putc('\n', stdout);
    for (isize i = 0; i <= gapbuf->capacity; i++) {
        if (i == gapbuf->gap_begin && i == gapbuf->gap_end) {
            putc('^', stdout);
        } else if (i == gapbuf->gap_begin) {
            putc('<', stdout);
        } else if (i == gapbuf->gap_end) {
            putc('>', stdout);
        } else {
            putc(' ', stdout);
        }
    }
    putc('\n', stdout);
}

void gapbuf_read_entire_file(GapBuffer* gapbuf, const char* filename) {
    gapbuf_clear(gapbuf);
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("Couldn't Open File: ");
        return;
    }
    isize len = string_get_file_length(f);
    gapbuf_expand(gapbuf, len);
    fread(gapbuf->data, 1, len, f);
    gapbuf->gap_begin = len;
    gapbuf->gap_end = gapbuf->capacity;
}
void gapbuf_write_entire_file(GapBuffer* gapbuf, const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        perror("Couldn't Open File: ");
        return;
    }
    GapBufSlice strings = gapbuf_getstrings(gapbuf);
    string_write_file(f, strings.l);
    string_write_file(f, strings.r);
    fclose(f);
}

#endif
#endif //GAPBUFFER_H_