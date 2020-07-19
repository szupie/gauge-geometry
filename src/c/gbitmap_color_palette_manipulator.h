/*
The MIT License (MIT)

Copyright (c) 2015 Jonathan LaRocque

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <pebble.h>

#ifdef PBL_COLOR
char* get_gbitmapformat_text(GBitmapFormat format);
const char* get_gcolor_text(GColor m_color);
void replace_gbitmap_color(GColor color_to_replace, GColor replace_with_color, GBitmap *im, BitmapLayer *bml);
void spit_gbitmap_color_palette(GBitmap *im);
bool gbitmap_color_palette_contains_color(GColor m_color, GBitmap *im);
void gbitmap_fill_all_except(GColor color_to_not_change, GColor fill_color, bool fill_gcolorclear, GBitmap *im, BitmapLayer *bml);
#endif