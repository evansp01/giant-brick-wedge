#include <stdint.h>

#define PAGES_PER_TABLE 1024
#define TABLES_PER_DIR 1024

typedef struct {
    uint32_t present: 1;             /* bit 0 */
    uint32_t read_write: 1;          /* bit 1 */
    uint32_t user_superuser: 1;      /* bit 2 */
    uint32_t write_through: 1;       /* bit 3 */
    uint32_t cache_disable: 1;       /* bit 4 */
    uint32_t accessed: 1;            /* bit 5 */
    uint32_t dirty: 1;               /* bit 6 */
    uint32_t page_size: 1;           /* bit 7 */
    uint32_t global: 1;              /* bit 8 */
    uint32_t available: 3;           /* bit 9  - 11 */
    uint32_t address: 20;            /* bit 12 - 32 */
} entry_t;

typedef struct {
    entry_t tables[TABLES_PER_DIR];
} page_directory_t;

typedef struct {
    entry_t pages[PAGES_PER_TABLE];
} page_table_t;
