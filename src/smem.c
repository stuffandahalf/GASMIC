// Wrapper for memory allocation functions
// Useful for clearing all allocated memory in the event of a failure

#include <smem.h>

struct alloced {
    void *address;
    struct alloced *next;
	struct alloced *prev;
};

static struct {
    struct alloced *first;
    struct alloced *last;
} alloced_mem = {
    .first = NULL,
    .last = NULL
};

void release(void) {
    struct alloced *a = alloced_mem.first;
    while (a != NULL) {
        struct alloced *tmp = a;
        a = a->next;
        free(tmp->address);
        free(tmp);
    }
    alloced_mem.first = NULL;
    alloced_mem.last = NULL;
}

static struct alloced *find_memory(void *ptr) {
    struct alloced *mem;
	if (ptr == alloced_mem.last->address) {
		return alloced_mem.last;
	}
    for (mem = alloced_mem.first; mem != NULL && mem != alloced_mem.last; mem = mem->next) {
        if (mem->address == ptr) {
            //break;
            return mem;
        }
    }
    //if (mem == NULL) {
    die("Failed to locate provided address, are you sure you salloced it?\n");
    //}
    //return mem;
}

void smem_diagnostic(void) {
    struct alloced *a = alloced_mem.first;
    while (a != NULL) {
        printf("%p\n", a->address);
        a = a->next;
    }
    puts("");
}

void *salloc(size_t size) {
    void *ptr = NULL;
    if ((ptr = malloc(size)) == NULL) {
        die("Failed to allocate memory\n");
    }
    
	saquire(ptr);
    
    return ptr;
}

void *saquire(void *ptr) {
	struct alloced *a;
	if ((a = malloc(sizeof(struct alloced))) == NULL) {
		die("Failed to allocate storage for allocated memory\n");
	}

	a->address = ptr;
	a->next = NULL;

	if (alloced_mem.first == NULL) {
		alloced_mem.first = a;
		a->prev = NULL;
	}
	else {
		alloced_mem.last->next = a;
		a->prev = alloced_mem.last;
	}
	alloced_mem.last = a;

	return a->address;
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
    /*struct alloced *prev = NULL;
    struct alloced *a = alloced_mem.first;
    //printf("looking for address: %p\n", ptr);
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
    
    if (alloced_mem.first == a && alloced_mem.last == a) {
        alloced_mem.first = NULL;
        alloced_mem.last = NULL;
    }
    else if (alloced_mem.first == a) {
        alloced_mem.first = a->next;
    }
    else if (alloced_mem.last == a) {
        alloced_mem.last = prev;
    }
    else {
        prev->next = a->next;
    }*/

	struct alloced *a = find_memory(ptr);

	struct alloced *prev = a->prev;
	struct alloced *next = a->next;

    if (prev != NULL) {
	    prev->next = next;
    }
    if (next != NULL) {
	    next->prev = prev;
    }
    
    free(a->address);
    free(a);
}
