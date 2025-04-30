#include <stdio.h>
#include <locale.h>
#include <stdint.h>
#include <wchar.h>

int is_identifier_start(uint32_t cp) {
  if (cp >= 0xD800 && cp <= 0xDFFF) return 0; // Surrogate halves
  if ((cp & 0xFFFF) == 0xFFFF || (cp & 0xFFFF) == 0xFFFE) return 0; // Noncharacters

  // ASCII letters and underscore
  if ((cp >= 'A' && cp <= 'Z') ||
      (cp >= 'a' && cp <= 'z') ||
      cp == '_') return 1;

  // Latin-1 Supplement
  if ((cp >= 0x00C0 && cp <= 0x00D6) ||
      (cp >= 0x00D8 && cp <= 0x00F6) ||
      (cp >= 0x00F8 && cp <= 0x00FF)) return 1;

  // Latin Extended-A and B
  if ((cp >= 0x0100 && cp <= 0x017F) ||
      (cp >= 0x0180 && cp <= 0x024F)) return 1;

  // IPA Extensions, Spacing Modifier Letters
  if ((cp >= 0x0250 && cp <= 0x02AF) ||
      (cp >= 0x02B0 && cp <= 0x02FF)) return 1;

  // Greek and Coptic
  if (cp >= 0x0370 && cp <= 0x03FF) return 1;

  // Cyrillic
  if ((cp >= 0x0400 && cp <= 0x04FF) ||
      (cp >= 0x0500 && cp <= 0x052F)) return 1;

  // Armenian
  if (cp >= 0x0531 && cp <= 0x0556) return 1;

  // Hebrew
  if (cp >= 0x05D0 && cp <= 0x05EA) return 1;

  // Arabic
  if ((cp >= 0x0620 && cp <= 0x063F) ||
      (cp >= 0x0641 && cp <= 0x064A)) return 1;

  // Syriac
  if (cp >= 0x0710 && cp <= 0x072F) return 1;

  // Thaana
  if (cp >= 0x0780 && cp <= 0x07BF) return 1;

  // Devanagari
  if (cp >= 0x0900 && cp <= 0x097F) return 1;

  // Bengali
  if (cp >= 0x0980 && cp <= 0x09FF) return 1;

  // Gurmukhi
  if (cp >= 0x0A00 && cp <= 0x0A7F) return 1;

  // Gujarati
  if (cp >= 0x0A80 && cp <= 0x0AFF) return 1;

  // Oriya
  if (cp >= 0x0B00 && cp <= 0x0B7F) return 1;

  // Tamil
  if (cp >= 0x0B80 && cp <= 0x0BFF) return 1;

  // Telugu
  if (cp >= 0x0C00 && cp <= 0x0C7F) return 1;

  // Kannada
  if (cp >= 0x0C80 && cp <= 0x0CFF) return 1;

  // Malayalam
  if (cp >= 0x0D00 && cp <= 0x0D7F) return 1;

  // Sinhala
  if (cp >= 0x0D80 && cp <= 0x0DFF) return 1;

  // Thai
  if (cp >= 0x0E00 && cp <= 0x0E7F) return 1;

  // Lao
  if (cp >= 0x0E80 && cp <= 0x0EFF) return 1;

  // Tibetan
  if (cp >= 0x0F00 && cp <= 0x0FBF) return 1;

  // Myanmar
  if (cp >= 0x1000 && cp <= 0x109F) return 1;

  // Georgian
  if ((cp >= 0x10A0 && cp <= 0x10FF) || (cp >= 0x2D00 && cp <= 0x2D2F)) return 1;

  // Hangul Jamo
  if (cp >= 0x1100 && cp <= 0x11FF) return 1;

  // Ethiopic
  if (cp >= 0x1200 && cp <= 0x137F) return 1;

  // Cherokee
  if (cp >= 0x13A0 && cp <= 0x13FF) return 1;

  // Unified Canadian Aboriginal Syllabics
  if (cp >= 0x1400 && cp <= 0x167F) return 1;

  // Ogham
  if (cp >= 0x1680 && cp <= 0x169F) return 1;

  // Runic
  if (cp >= 0x16A0 && cp <= 0x16FF) return 1;

  // Khmer
  if (cp >= 0x1780 && cp <= 0x17FF) return 1;

  // Mongolian
  if (cp >= 0x1800 && cp <= 0x18AF) return 1;

  // Limbu
  if (cp >= 0x1900 && cp <= 0x194F) return 1;

  // Tai Le
  if (cp >= 0x1950 && cp <= 0x197F) return 1;

  // New Tai Lue
  if (cp >= 0x1980 && cp <= 0x19DF) return 1;

  // Bopomofo
  if ((cp >= 0x3100 && cp <= 0x312F) ||
      (cp >= 0x31A0 && cp <= 0x31BF)) return 1;

  // CJK Unified Ideographs (Chinese, Japanese, Korean)
  if ((cp >= 0x4E00 && cp <= 0x9FFF) ||
      (cp >= 0x3400 && cp <= 0x4DBF) || // Extension A
      (cp >= 0x20000 && cp <= 0x2A6DF) || // Extension B
      (cp >= 0x2A700 && cp <= 0x2B73F) || // Extension C
      (cp >= 0x2B740 && cp <= 0x2B81F) || // Extension D
      (cp >= 0x2B820 && cp <= 0x2CEAF) || // Extension E
      (cp >= 0x2CEB0 && cp <= 0x2EBEF)) // Extension F
    return 1;

  // Hiragana, Katakana (Japanese)
  if ((cp >= 0x3040 && cp <= 0x309F) || (cp >= 0x30A0 && cp <= 0x30FF)) return 1;

  // Yi Syllables
  if (cp >= 0xA000 && cp <= 0xA48F) return 1;

  // Letter Numbers (like Roman numerals)
  if (cp >= 0x2160 && cp <= 0x2188) return 1;

  // Modifier letters
  if ((cp >= 0x02B0 && cp <= 0x02FF)) return 1;

  return 0;
}

int main(void) {
  setlocale(LC_ALL, "");

  for (uint32_t cp = 0; cp <= 0x10FFFF; ++cp) {
    if (is_identifier_start(cp)) {
      if (cp <= 0xFFFF) {
        wprintf(L"%lc", (wchar_t)cp);
      } else {
        // Output surrogate pairs if necessary
        uint32_t high = ((cp - 0x10000) >> 10) + 0xD800;
        uint32_t low  = ((cp - 0x10000) & 0x3FF) + 0xDC00;
        wprintf(L"%lc%lc", (wchar_t)high, (wchar_t)low);
      }
    }
  }

  return 0;
}
