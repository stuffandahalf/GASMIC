#include <smem.h>

struct alloced {
    void *address;
    struct alloced *next;
};

static struct {
    struct alloced *first;
    struct alloced *last;
} alloced_mem;

void release() {
    struct alloced *a = alloced_mem.first;
    while (a != NULL) {
        free(a->address);
        struct alloced *tmp = a;
        a = a->next;
        free(tmp);
    }
    alloced_mem.first = NULL;
    alloced_mem.last = NULL;
}

static struct alloced *find_memory(void *ptr) {
    struct alloced *mem;
    for (mem = alloced_mem.first; mem != NULL; mem = mem->next) {
        if (mem->address == ptr) {
            break;
        }
    }
    if (mem == NULL) {
        die("Failed to locate provided address, are you sure you salloced it?\n");
    }
    return mem;
}

static void diagnostic() {
    struct alloced *a = alloced_mem.first;
    while (a != NULL) {
        printf("%p\t", a->address);
        a = a->next;
    }
    puts("");
}

void *salloc(size_t size) {
    void *tmp;
    if ((tmp = malloc(size)) == NULL) {
        die("Failed to allocate memory\n");
    }
    
    struct alloced *a;
    if ((a = malloc(sizeof(struct alloced))) == NULL) {
        die("failed to allocate storage for alloced memory\n");
    }
    
    a->address = tmp;
    a->next = NULL;
    
    if (alloced_mem.first == NULL) {
        alloced_mem.first = a;
    }
    else {
        alloced_mem.last->next = a;
    }
    alloced_mem.last = a;
    
    return tmp;
}

void *srealloc(void *ptr, size_t size) {
    struct alloced *a = find_memory(ptr);
    
    if ((ptr = realloc(ptr, size)) == NULL) {
        die("Failed to reallocate memory\n");
    }
    
    a->address = ptr;
    return ptr;
}

void sfree(void *ptr) {
    struct alloced *prev = NULL;
    struct alloced *a = alloced_mem.first;
    printf("lookinf for address: %p\n", ptr);
    while (a != NULL) {
        if (a->address == ptr) {
            break;
        }
        prev = a;
        a = a->next;
    }
    if (a == NULL || a->address != ptr) {
        die("Failed to locate the given address. are you sure it was salloced?\n");
    }
    
    if (alloced_mem.first == a) {
        if (alloced_mem.last == a) {
            alloced_mem.last = NULL;
        }
        alloced_mem.first = a->next;
    }
    else {
        prev->next = a->next;
    }
    
    free(a->address);
    free(a);
}