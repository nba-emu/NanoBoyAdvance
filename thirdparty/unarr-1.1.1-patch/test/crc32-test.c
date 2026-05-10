#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "../common/crc32.c"

static void crc32_9(void **state) {
    assert_int_equal(0xCBF43926, ar_crc32(0, "123456789", 9));
}

static void crc32_8(void **state) {
    assert_int_equal(0x9AE0DAAF, ar_crc32(0, "12345678", 8));
}

static void crc32_4(void **state) {
    assert_int_equal(0x9BE3E0A3, ar_crc32(0, "1234", 4));
}

static void crc32_2(void **state) {
    assert_int_equal(0x4F5344CD, ar_crc32(0, "12", 2));
}

static void crc32_1(void **state) {
    assert_int_equal(0x83DCEFB7, ar_crc32(0, "1", 1));
}

static void crc32_1_0(void **state) {
    assert_int_equal(0xD202EF8D, ar_crc32(0, "\0", 1));
}

static void crc32_1_FF(void **state) {
    assert_int_equal(0xFF000000, ar_crc32(0, "\xFF", 1));
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(crc32_9),
        cmocka_unit_test(crc32_8),
        cmocka_unit_test(crc32_4),
        cmocka_unit_test(crc32_2),
        cmocka_unit_test(crc32_1),
        cmocka_unit_test(crc32_1_0),
        cmocka_unit_test(crc32_1_FF)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
