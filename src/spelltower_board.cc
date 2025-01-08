#include "spelltower_board.h"

namespace puzzmo {

const char SpelltowerBoard::LetterAt(Point p) { return board_[p.row][p.col]; }

} // namespace puzzmo