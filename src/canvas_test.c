#include "minunit.h"
#include "canvas.h"

static int cols = 2;
static int rows = 3;
static char load_str[] = "012345";

static Canvas *c1, *c2;

MU_TEST(test_canvas_new) {
    c1 = canvas_new(cols, rows);
    mu_assert(c1->num_cols == cols, "cols should be 2");
    mu_assert(c1->num_rows == rows, "rows should be 3");
    canvas_free(c1);
}

MU_TEST(test_canvas_load_str) {
    c1 = canvas_new(cols, rows);
    int numset = canvas_load_str(c1, load_str);
    mu_assert(numset == sizeof(load_str) - 1, "numset should be 6");
    mu_check(c1->rows[0][0] == '0');
    mu_check(c1->rows[0][1] == '1');
    mu_check(c1->rows[1][0] == '2');
    mu_check(c1->rows[1][1] == '3');
    mu_check(c1->rows[2][0] == '4');
    mu_check(c1->rows[2][1] == '5');

    canvas_free(c1);
}

MU_TEST(test_canvas_eq) {
    // canvas with same content
    c1 = canvas_new(cols, rows);
    c2 = canvas_new(cols, rows);

    // canvases of different sizes
    Canvas *c3 = canvas_new(cols - 1, rows);
    Canvas *c4 = canvas_new(cols, rows - 1);

    // init all canvases with same str
    canvas_load_str(c1, load_str);
    canvas_load_str(c2, load_str);
    canvas_load_str(c3, load_str);
    canvas_load_str(c4, load_str);

    // same canvases should be the same
    mu_check(canvas_eq(c1, c2));
    mu_check(canvas_eq(c2, c1));

    // different canvases should be different
    mu_check(!canvas_eq(c1, c3));
    mu_check(!canvas_eq(c1, c4));
    mu_check(!canvas_eq(c3, c4));

    // change one of the same canvases
    c1->rows[2][1] = 'A';
    mu_check(!canvas_eq(c1, c2));

    // change the other to match
    c2->rows[2][1] = 'A';
    mu_check(canvas_eq(c1, c2));

    canvas_free(c1);
    canvas_free(c2);
    canvas_free(c3);
    canvas_free(c4);
}

// Tests to make sure the setup/teardown and eq functions will work
MU_TEST_SUITE(canvas_bootstrapping) {
    MU_RUN_TEST(test_canvas_new);
    MU_RUN_TEST(test_canvas_load_str);
    MU_RUN_TEST(test_canvas_eq);
}

void test_setup(void) {
    c1 = canvas_new(cols, rows);
    canvas_load_str(c1, load_str);
}

void test_teardown(void) {
	canvas_free(c1);
}

MU_TEST(test_canvas_gcharxy) {
    mu_check(c1->rows[2][1] == canvas_gcharxy(c1, 1, 2));
    mu_check(c1->rows[2][1] != canvas_gcharxy(c1, 0, 2));
}

MU_TEST(test_canvas_gchari) {
    mu_check(c1->rows[2][1] == canvas_gchari(c1, 5));
    mu_check(c1->rows[2][1] != canvas_gchari(c1, 4));
}

MU_TEST(test_canvas_scharxy) {
    mu_check(c1->rows[0][0] != 'X');
    canvas_scharxy(c1, 0, 0, 'X');
    mu_check(c1->rows[0][0] == 'X');

    mu_check(c1->rows[2][1] != 'X');
    canvas_scharxy(c1, 1, 2, 'X');
    mu_check(c1->rows[2][1] == 'X');
}

MU_TEST(test_canvas_schari) {
    mu_check(canvas_gchari(c1, 0) != 'X');
    canvas_schari(c1, 0, 'X');
    mu_check(canvas_gchari(c1, 0) == 'X');

    mu_check(canvas_gchari(c1, 4) != 'X');
    canvas_schari(c1, 4, 'X');
    mu_check(canvas_gchari(c1, 4) == 'X');
}

MU_TEST(test_canvas_cpy) {
    c2 = canvas_cpy(c1);

    mu_assert(c1 != c2, "Canvas pointers should not be the same");
    mu_assert(canvas_eq(c1, c2), "Canvases should be equal");
    canvas_schari(c2, 5, 'X');
    mu_assert(!canvas_eq(c1, c2), "Canvases should no longer be equal");

    canvas_free(c2);
}

MU_TEST(test_canvas_serialize_deserialize) {
    char buf[c1->num_rows * c1->num_cols];
    int numwritten = canvas_serialize(c1, buf);
    mu_assert(numwritten == sizeof(buf), "Serialized form should be size cols*rows");

    c2 = canvas_new(c1->num_cols, c1->num_rows);
    canvas_deserialize(buf, c2);
    mu_assert(canvas_eq(c1, c2), "Canvases should be equivalent");

    canvas_free(c2);
}

// test the rest of canvas functions
MU_TEST_SUITE(canvas_main) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    MU_RUN_TEST(test_canvas_gcharxy);
    MU_RUN_TEST(test_canvas_gchari);
    MU_RUN_TEST(test_canvas_scharxy);
    MU_RUN_TEST(test_canvas_schari);

    MU_RUN_TEST(test_canvas_cpy);

    MU_RUN_TEST(test_canvas_serialize_deserialize);
}

int main(int argc, char const *argv[])
{
    MU_RUN_SUITE(canvas_bootstrapping);
    MU_RUN_SUITE(canvas_main);
    MU_REPORT();
    return minunit_status;
}
