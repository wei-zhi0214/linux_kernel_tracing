// memhog.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static unsigned long long parse_bytes(const char *s) {
    char *end = NULL;
    unsigned long long val = strtoull(s, &end, 10);
    if (*end == 'K' || *end == 'k') val *= 1024ULL;
    else if (*end == 'M' || *end == 'm') val *= 1024ULL*1024ULL;
    else if (*end == 'G' || *end == 'g') val *= 1024ULL*1024ULL*1024ULL;
    return val;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <bytes|K|M|G>\n", argv[0]);
        return 1;
    }
    unsigned long long need = parse_bytes(argv[1]);
    char *p = malloc(need);
    if (!p) {
        perror("malloc");
        return 2;
    }
    // 觸碰每個頁面，確保實配
    for (unsigned long long i = 0; i < need; i += 4096ULL) p[i] = 1;
    printf("Allocated and touched %llu bytes. Sleeping...\n", need);
    sleep(30);
    return 0;
}

