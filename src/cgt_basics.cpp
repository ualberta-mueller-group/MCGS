#include "cgt_basics.h"

static bool initialized = false;

static int char_to_color_table[NUM_MAX_COLORS];
static char color_to_char_table[NUM_MAX_COLORS];


namespace {
void add_color(int color_int, char color_char)
{
    assert(0 <= color_int && color_int < NUM_MAX_COLORS);

    static_assert(std::numeric_limits<unsigned char>::min() == 0 &&
                  std::numeric_limits<unsigned char>::max() ==
                      NUM_MAX_COLORS - 1);

    const unsigned char& color_char_unsigned =
        reinterpret_cast<const unsigned char&>(color_char);

    // add color -> char
    assert(color_to_char_table[color_int] == CHAR_INVALID);
    color_to_char_table[color_int] = color_char;

    // add char -> color
    assert(char_to_color_table[color_char_unsigned] == COLOR_INVALID);
    char_to_color_table[color_char_unsigned] = color_int;
}
} // namespace


namespace __color_impl { // NOLINT(readability-identifier-naming)
const char* get_color_to_char_table()
{
    assert(initialized);
    return color_to_char_table;
}

const int* get_char_to_color_table()
{
    assert(initialized);
    return char_to_color_table;
}

} // namespace __color_impl

namespace mcgs_init {
void init_color_tables()
{
    assert(!initialized);

    for (int i = 0; i < NUM_MAX_COLORS; i++)
    {
        char_to_color_table[i] = COLOR_INVALID;
        color_to_char_table[i] = '?';
    }

    add_color(BLACK, 'X');
    add_color(WHITE, 'O');
    add_color(EMPTY, '.');
    add_color(BORDER, '#');
    add_color(ROW_SEP, '|');

    // Add invalid color last to ensure no collisions
    add_color(COLOR_INVALID, '?');

    initialized = true;
}

} // namespace mcgs_init

