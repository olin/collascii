#include "canvas.h"

int main() {
    // creating
    Canvas* c = canvas_new(3, 3);
    canvas_load_str(c, "X XXXXX X");
    canvas_print(c);

    Canvas* c_cpy = canvas_cpy(c);


    // setting
    printf("Set (1, 2) to 'O'\n");
    canvas_scharxy(c, 1, 2, 'O');
    canvas_print(c);
    printf("Equivalent: %s\n", canvas_eq(c_cpy, c) ? "True" : "False");

    // serialization
    printf("Original:\n");
    canvas_print(c);
    char buffer[10];
    canvas_serialize(c, buffer);
    buffer[9] = '\0';
    printf("Serialized:\n");
    printf("'%s'\n", buffer);
    Canvas* c2 = canvas_new(3,3);
    canvas_deserialize(buffer, c2);
    printf("Deserialized:\n");
    canvas_print(c2);
    printf("Equivalent: %s\n", canvas_eq(c, c2) ? "True" : "False");

    // free
    canvas_free(c);
    canvas_free(c2);
    return 0;
}