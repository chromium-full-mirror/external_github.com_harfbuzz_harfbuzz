/*
 * Copyright © 2026  Behdad Esfahbod
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "hb.hh"
#include "hb-ot-layout-gpos-table.hh"

using namespace OT::Layout::GPOS_impl;

static void
test_chain (hb_direction_t direction, bool backward)
{
  /* Exercise a whole-buffer chain beyond the recursion limits used by
   * other attachment walkers, in both attachment directions. */
  hb_glyph_position_t pos[257] = {};
  unsigned len = ARRAY_LENGTH (pos);
  for (unsigned i = 0; i < len; i++)
  {
    pos[i].x_offset = 2;
    pos[i].y_offset = 3;
    pos[i].attach_type() = ATTACH_TYPE_CURSIVE;
    pos[i].attach_chain() = backward ? (i ? -1 : 0) : (i + 1 < len ? 1 : 0);
  }

  for (unsigned i = 0; i < len; i++)
  {
    unsigned count = backward ? i + 1 : len - i;
    hb_position_t expected = count * (HB_DIRECTION_IS_HORIZONTAL (direction) ? 3 : 2);
    hb_always_assert (resolve_cross_offset (pos, len, i, direction) == expected);
    hb_always_assert (pos[i].attach_chain() ==
                      (backward ? (i ? -1 : 0) : (i + 1 < len ? 1 : 0)));
  }
}

static void
test_cycle (hb_direction_t direction)
{
  hb_glyph_position_t pos[3] = {};
  for (unsigned i = 0; i < ARRAY_LENGTH (pos); i++)
  {
    pos[i].x_offset = 2;
    pos[i].y_offset = 3;
    pos[i].attach_type() = ATTACH_TYPE_CURSIVE;
    pos[i].attach_chain() = i == 2 ? -2 : 1;
  }
  for (unsigned i = 0; i < ARRAY_LENGTH (pos); i++)
    hb_always_assert (resolve_cross_offset (pos, ARRAY_LENGTH (pos), i, direction) ==
                      (HB_DIRECTION_IS_HORIZONTAL (direction) ? 9 : 6));
}

static void
test_roots (hb_direction_t direction)
{
  hb_glyph_position_t pos[2] = {};
  pos[0].x_offset = 7;
  pos[0].y_offset = 11;
  pos[0].attach_type() = ATTACH_TYPE_CURSIVE;
  pos[1].x_offset = 101;
  pos[1].y_offset = 103;
  hb_position_t expected = HB_DIRECTION_IS_HORIZONTAL (direction) ? 11 : 7;
  /* A zero chain and either out-of-range parent stop at this glyph. */
  for (int chain : {0, -1, 2})
  {
    pos[0].attach_chain() = chain;
    hb_always_assert (resolve_cross_offset (pos, 2, 0, direction) == expected);
  }
  /* A non-cursive attachment does not continue the walk. */
  pos[0].attach_chain() = 1;
  pos[0].attach_type() = ATTACH_TYPE_MARK;
  hb_always_assert (resolve_cross_offset (pos, 2, 0, direction) == expected);
}

int
main ()
{
  for (hb_direction_t direction : {HB_DIRECTION_LTR, HB_DIRECTION_RTL,
                                   HB_DIRECTION_TTB, HB_DIRECTION_BTT})
  {
    test_chain (direction, false);
    test_chain (direction, true);
    test_roots (direction);
    test_cycle (direction);
  }
  return 0;
}
