#include <stdio.h>
#include <stdlib.h>

// declare external test entry functions
extern int run_load_unload_test(void);
extern int run_read_write_test(void);
extern int run_read_write_threaded(void);
extern int run_priority_order_test(void);

int main(void) {
    int ret = 0;

    printf("Running load/unload test...\n");
    ret = run_load_unload_test();
    if (ret) { printf("❌ load/unload test failed\n"); return ret; }
    printf("✅ load/unload test passed\n\n");

    printf("Running read/write test...\n");
    ret = run_read_write_test();
    if (ret) { printf("❌ read/write test failed\n"); return ret; }
    printf("✅ read/write test passed\n\n");

    printf("Running read/write threaded test...\n");
    ret = run_read_write_threaded();
    if (ret) { printf("❌ threaded test failed\n"); return ret; }
    printf("✅ threaded test passed\n\n");

    printf("Running priority order test...\n");
    ret = run_priority_order_test();
    if (ret) { printf("❌ priority order test failed\n"); return ret; }
    printf("✅ priority order test passed\n\n");

    printf("🎉 All tests completed successfully!\n");
    return 0;
}
