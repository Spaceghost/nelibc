#define _GNU_SOURCE
#include <errno.h>
#include <locale.h>
#include <langinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static void print_wide(const wchar_t *text, size_t count) {
  for (size_t i = 0; i < count; ++i) printf("%x,", (unsigned)text[i]);
  putchar(';');
}

static void print_bytes(const char *text, size_t count) {
  for (size_t i = 0; i < count; ++i) printf("%02x,", (unsigned char)text[i]);
  putchar(';');
}

static void run_legacy_locale(void) {
  if (setlocale(LC_CTYPE, "en_US.iso88591") == NULL) {
    puts("legacy-locale:skip");
    return;
  }
  const char *codeset = nl_langinfo(CODESET);
  if (strcmp(codeset, "ISO-8859-1") != 0 && strcmp(codeset, "ISO8859-1") != 0) {
    printf("legacy-locale:skip:%s\n", codeset);
    return;
  }
  const char bytes[] = {(char)0xe9, 0};
  const wchar_t wide_text[] = {0xe9, 0};
  wchar_t wide = 0;
  char encoded[8] = {0};
  mblen(NULL, 0);
  mbtowc(NULL, NULL, 0);
  errno = 0;
  int single_length = mblen(bytes, 1);
  int single_conversion = mbtowc(&wide, bytes, 1);
  wchar_t wide_output[2] = {0};
  errno = 0;
  size_t string_decode = mbstowcs(wide_output, bytes, 2);
  int string_decode_errno = errno;
  errno = 0;
  size_t string_decode_query = mbstowcs(NULL, bytes, 0);
  int string_decode_query_errno = errno;
  char byte_output[4] = {0};
  errno = 0;
  size_t string_encode = wcstombs(byte_output, wide_text, sizeof byte_output);
  int string_encode_errno = errno;
  errno = 0;
  size_t string_encode_query = wcstombs(NULL, wide_text, 0);
  int string_encode_query_errno = errno;
  printf("legacy-nonrestart:%d,%d,%x,%lld,%d,%x,%x,%lld,%d,%02x,%02x,%lld,%d\n",
      single_length, single_conversion, (unsigned)wide, (long long)string_decode,
      string_decode_errno, (unsigned)wide_output[0], (unsigned)wide_output[1],
      (long long)string_decode_query, string_decode_query_errno,
      (unsigned char)byte_output[0], (unsigned char)byte_output[1],
      (long long)string_encode, string_encode_errno);
  printf("legacy-nonrestart-query:%lld,%d\n", (long long)string_encode_query,
      string_encode_query_errno);
  const wchar_t invalid_wide[][3] = {{L'A', 0xd800, 0}, {L'A', 0x110000, 0}};
  for (size_t invalid = 0; invalid < 2; ++invalid) {
    char invalid_output[4];
    memset(invalid_output, 0x77, sizeof invalid_output);
    errno = 0;
    size_t invalid_count = wcstombs(invalid_output, invalid_wide[invalid],
        sizeof invalid_output);
    int invalid_errno = errno;
    errno = 0;
    size_t invalid_query = wcstombs(NULL, invalid_wide[invalid], 0);
    printf("legacy-invalid-wide:%zu,%lld,%d,%02x,%02x,%lld,%d\n", invalid,
        (long long)invalid_count, invalid_errno, (unsigned char)invalid_output[0],
        (unsigned char)invalid_output[1], (long long)invalid_query, errno);
  }

  mbstate_t decode_state = {0};
  mbstate_t encode_state = {0};
  errno = 0;
  size_t decoded = mbrtowc(&wide, bytes, 1, &decode_state);
  int decode_errno = errno;
  errno = 0;
  size_t encoded_count = wcrtomb(encoded, wide, &encode_state);
  printf("legacy-char:%lld,%d,%x,%lld,%d,%02x,%x,%d\n",
      (long long)decoded, decode_errno, (unsigned)wide, (long long)encoded_count,
      errno, (unsigned char)encoded[0], (unsigned)btowc(0xe9), wctob(0xe9));

  const char *decode_cursor = bytes;
  wide_output[0] = 0;
  wide_output[1] = 0;
  mbstate_t restart_decode_state = {0};
  errno = 0;
  size_t wide_count = mbsrtowcs(wide_output, &decode_cursor, 2,
      &restart_decode_state);
  int wide_errno = errno;
  const wchar_t *encode_cursor = wide_text;
  byte_output[0] = 0;
  byte_output[1] = 0;
  byte_output[2] = 0;
  byte_output[3] = 0;
  mbstate_t restart_encode_state = {0};
  errno = 0;
  size_t byte_count = wcsrtombs(byte_output, &encode_cursor, sizeof byte_output,
      &restart_encode_state);
  printf("legacy-string:%lld,%d,%x,%d,%lld,%d,%02x,%d\n",
      (long long)wide_count, wide_errno, (unsigned)wide_output[0],
      decode_cursor == NULL, (long long)byte_count, errno,
      (unsigned char)byte_output[0], encode_cursor == NULL);
}

static void run_stateful_locale(void) {
  if (setlocale(LC_CTYPE, "ja_JP.eucjp") == NULL) {
    puts("stateful-locale:skip");
    return;
  }
  const char *codeset = nl_langinfo(CODESET);
  if (strcmp(codeset, "EUC-JP") != 0 && strcmp(codeset, "EUCJP") != 0) {
    printf("stateful-locale:skip:%s\n", codeset);
    return;
  }
  const char encoded_character[] = {(char)0xa4, (char)0xa2, 0};
  const wchar_t wide_character[] = {0x3042, 0};
  mbstate_t decode_state = {0};
  wchar_t decoded = 0;
  errno = 0;
  size_t first = mbrtowc(&decoded, encoded_character, 1, &decode_state);
  int first_errno = errno;
  int pending_initial = mbsinit(&decode_state);
  errno = 0;
  size_t second = mbrtowc(&decoded, encoded_character + 1, 1, &decode_state);
  printf("eucjp-split:%lld,%d,%d,%lld,%d,%x,%d\n", (long long)first,
      first_errno, pending_initial, (long long)second, errno, (unsigned)decoded,
      mbsinit(&decode_state));

  mbstate_t recovery_state = {0};
  wchar_t recovery_wide = 0;
  errno = 0;
  size_t recovery_prefix = mbrtowc(&recovery_wide, encoded_character, 1,
      &recovery_state);
  int recovery_prefix_errno = errno;
  errno = 0;
  size_t recovery_error = mbrtowc(&recovery_wide, "A", 1, &recovery_state);
  int recovery_error_errno = errno;
  recovery_state = (mbstate_t){0};
  int recovery_initial = mbsinit(&recovery_state);
  errno = 0;
  size_t recovery_valid = mbrtowc(&recovery_wide, encoded_character, 2,
      &recovery_state);
  printf("eucjp-recovery:%lld,%d,%lld,%d,%d,%lld,%x,%d\n",
      (long long)recovery_prefix, recovery_prefix_errno, (long long)recovery_error,
      recovery_error_errno, recovery_initial, (long long)recovery_valid,
      (unsigned)recovery_wide, errno);

  mbstate_t null_state = {0};
  size_t null_prefix = mbrtowc(&recovery_wide, encoded_character, 1, &null_state);
  errno = 0;
  size_t null_pending = mbrtowc(NULL, NULL, 0, &null_state);
  printf("eucjp-null-pending:%lld,%lld,%d\n", (long long)null_prefix,
      (long long)null_pending, errno);

  mbstate_t length_state = {0};
  size_t length_first = mbrlen(encoded_character, 1, &length_state);
  int length_pending_initial = mbsinit(&length_state);
  size_t length_second = mbrlen(encoded_character + 1, 1, &length_state);
  printf("eucjp-mbrlen:%lld,%d,%lld,%d\n", (long long)length_first,
      length_pending_initial, (long long)length_second, mbsinit(&length_state));

  mbstate_t encode_state = {0};
  char encoded[8] = {0};
  errno = 0;
  size_t encode_count = wcrtomb(encoded, 0x3042, &encode_state);
  printf("eucjp-encode:%lld,%d,", (long long)encode_count, errno);
  if (encode_count != (size_t)-1) print_bytes(encoded, encode_count);
  else putchar(';');

  const char *decode_cursor = encoded_character;
  wchar_t wide_output[2] = {0};
  mbstate_t restart_decode_state = {0};
  errno = 0;
  size_t wide_count = mbsrtowcs(wide_output, &decode_cursor, 2,
      &restart_decode_state);
  int wide_errno = errno;
  const wchar_t *encode_cursor = wide_character;
  char byte_output[8] = {0};
  mbstate_t restart_encode_state = {0};
  errno = 0;
  size_t byte_count = wcsrtombs(byte_output, &encode_cursor, sizeof byte_output,
      &restart_encode_state);
  printf("eucjp-strings:%lld,%d,%x,%d,%lld,%d,", (long long)wide_count,
      wide_errno, (unsigned)wide_output[0], decode_cursor == NULL,
      (long long)byte_count, errno);
  if (byte_count != (size_t)-1) print_bytes(byte_output, byte_count + 1);
  else putchar(';');

  const char *bounded_cursor = encoded_character;
  wchar_t bounded_output[2] = {0};
  mbstate_t bounded_state = {0};
  size_t bounded_first = mbsnrtowcs(bounded_output, &bounded_cursor, 1, 2,
      &bounded_state);
  const char *query_cursor = bounded_cursor;
  size_t bounded_query = mbsrtowcs(NULL, &query_cursor, 0, &bounded_state);
  int bounded_pending_initial = mbsinit(&bounded_state);
  size_t bounded_second = mbsnrtowcs(bounded_output, &bounded_cursor, 2, 2,
      &bounded_state);
  printf("eucjp-bounded:%lld,%lld,%d,%d,%lld,%x,%d\n",
      (long long)bounded_first, (long long)bounded_query,
      query_cursor == encoded_character + 1, bounded_pending_initial,
      (long long)bounded_second, (unsigned)bounded_output[0], bounded_cursor == NULL);

  const char invalid_bytes[][3] = {{'A', (char)0xff, 0}, {'A', (char)0xa4, 0}};
  for (size_t invalid = 0; invalid < 2; ++invalid) {
    wchar_t invalid_output[3] = {0x777, 0x777, 0x777};
    errno = 0;
    size_t invalid_count = mbstowcs(invalid_output, invalid_bytes[invalid], 3);
    int invalid_errno = errno;
    wchar_t limited_output[3] = {0x777, 0x777, 0x777};
    errno = 0;
    size_t limited_count = mbstowcs(limited_output, invalid_bytes[invalid], 1);
    int limited_errno = errno;
    errno = 0;
    size_t invalid_query = mbstowcs(NULL, invalid_bytes[invalid], 0);
    printf("eucjp-invalid-mbstowcs:%zu,%lld,%d,%x,%x,%lld,%d,%x,%d,%lld,%d\n",
        invalid, (long long)invalid_count, invalid_errno,
        (unsigned)invalid_output[0], (unsigned)invalid_output[1],
        (long long)limited_count, limited_errno, (unsigned)limited_output[0],
        limited_output[1] == 0x777, (long long)invalid_query, errno);
  }

  for (size_t invalid = 0; invalid < 2; ++invalid) {
    const char *invalid_cursor = invalid_bytes[invalid];
    wchar_t invalid_wide[3] = {0x777, 0x777, 0x777};
    mbstate_t invalid_state = {0};
    errno = 0;
    size_t invalid_count = mbsrtowcs(invalid_wide, &invalid_cursor, 3,
        &invalid_state);
    int invalid_errno = errno;
    ptrdiff_t invalid_offset = invalid_cursor - invalid_bytes[invalid];
    const char *query_cursor = invalid_bytes[invalid];
    mbstate_t query_state = {0};
    errno = 0;
    size_t invalid_query = mbsrtowcs(NULL, &query_cursor, 0, &query_state);
    int query_errno = errno;
    const char *limited_cursor = invalid_bytes[invalid];
    wchar_t limited_wide[2] = {0x777, 0x777};
    mbstate_t limited_state = {0};
    errno = 0;
    size_t limited_count = mbsrtowcs(limited_wide, &limited_cursor, 1,
        &limited_state);
    printf("eucjp-invalid-mbsrtowcs:%zu,%lld,%d,%x,%d,%td,%lld,%d,%d,%lld,%d,%x\n",
        invalid, (long long)invalid_count, invalid_errno, (unsigned)invalid_wide[0],
        invalid_wide[1] == 0x777, invalid_offset, (long long)invalid_query,
        query_errno, query_cursor == invalid_bytes[invalid], (long long)limited_count,
        errno, (unsigned)limited_wide[0]);
  }

  for (size_t invalid = 0; invalid < 2; ++invalid) {
    const char *invalid_cursor = invalid_bytes[invalid];
    wchar_t invalid_wide[3] = {0x777, 0x777, 0x777};
    mbstate_t invalid_state = {0};
    errno = 0;
    size_t invalid_count = mbsnrtowcs(invalid_wide, &invalid_cursor, 3, 3,
        &invalid_state);
    int invalid_errno = errno;
    ptrdiff_t invalid_offset = invalid_cursor - invalid_bytes[invalid];
    const char *query_cursor = invalid_bytes[invalid];
    mbstate_t query_state = {0};
    errno = 0;
    size_t invalid_query = mbsnrtowcs(NULL, &query_cursor, 3, 0, &query_state);
    int query_errno = errno;
    const char *limited_cursor = invalid_bytes[invalid];
    wchar_t limited_wide[2] = {0x777, 0x777};
    mbstate_t limited_state = {0};
    errno = 0;
    size_t limited_count = mbsnrtowcs(limited_wide, &limited_cursor, 3, 1,
        &limited_state);
    printf("eucjp-invalid-mbsnrtowcs:%zu,%lld,%d,%x,%d,%td,%lld,%d,%d,%lld,%d,%td\n",
        invalid, (long long)invalid_count, invalid_errno, (unsigned)invalid_wide[0],
        invalid_wide[1] == 0x777, invalid_offset, (long long)invalid_query,
        query_errno, query_cursor == invalid_bytes[invalid], (long long)limited_count,
        errno, limited_cursor - invalid_bytes[invalid]);
    if (invalid == 1) {
      const char *partial_cursor = invalid_bytes[invalid];
      mbstate_t partial_state = {0};
      errno = 0;
      size_t partial_query = mbsnrtowcs(NULL, &partial_cursor, 2, 0,
          &partial_state);
      printf("eucjp-partial-query:%lld,%d,%d,%d\n", (long long)partial_query,
          errno, partial_cursor == invalid_bytes[invalid], mbsinit(&partial_state));
    }
  }

  const wchar_t *bounded_wide_cursor = wide_character;
  mbstate_t bounded_encode_state = {0};
  char bounded_bytes[8];
  memset(bounded_bytes, 0x77, sizeof bounded_bytes);
  size_t encode_query = wcsnrtombs(NULL, &bounded_wide_cursor, 1, 0,
      &bounded_encode_state);
  size_t too_small = wcsnrtombs(bounded_bytes, &bounded_wide_cursor, 2, 1,
      &bounded_encode_state);
  int unchanged_initial = mbsinit(&bounded_encode_state);
  size_t fits = wcsnrtombs(bounded_bytes, &bounded_wide_cursor, 2, 2,
      &bounded_encode_state);
  int advanced = bounded_wide_cursor == wide_character + 1;
  size_t terminator = wcsnrtombs(bounded_bytes + 2, &bounded_wide_cursor, 1, 1,
      &bounded_encode_state);
  printf("eucjp-wcsn:%lld,%d,%lld,%d,%d,%lld,%d,%02x,%02x,%02x,%d\n",
      (long long)encode_query, bounded_wide_cursor == NULL, (long long)too_small,
      unchanged_initial, advanced, (long long)fits, (int)terminator,
      (unsigned char)bounded_bytes[0], (unsigned char)bounded_bytes[1],
      (unsigned char)bounded_bytes[2], bounded_wide_cursor == NULL);
}

static void run_implicit_mbrlen(void) {
  if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) {
    puts("implicit-mbrlen:skip");
    return;
  }
  errno = 0;
  size_t prefix = mbrlen("\xc3", 1, NULL);
  int prefix_errno = errno;
  errno = 0;
  size_t null_input = mbrlen(NULL, 0, NULL);
  printf("implicit-mbrlen:%lld,%d,%lld,%d\n", (long long)prefix, prefix_errno,
      (long long)null_input, errno);
}

static void run_locale(const char *name, int utf8) {
  const char utf8_text[] = {'A', (char)0xc3, (char)0xa9, 'B', 0};
  const char truncated_text[] = {(char)0xe2, (char)0x82, 0};
  const char invalid_text[] = {(char)0xc3, 'X', 0};
  const wchar_t wide_text[] = {L'A', 0xe9, L'B', 0};
  const wchar_t supplementary_text[] = {0x1f642, 0};
  const wchar_t invalid_wide[] = {0xd800, 0};

  printf("locale:%s\n", name);
  mblen(NULL, 0);
  mbtowc(NULL, NULL, 0);
  wctomb(NULL, 0);
  wchar_t character = 0;
  errno = 0;
  int ascii_length = mblen("A", 1);
  int ascii_conversion = mbtowc(&character, "A", 1);
  unsigned ascii_character = (unsigned)character;
  mblen(NULL, 0);
  mbtowc(NULL, NULL, 0);
  int multibyte_length = -99;
  int multibyte_conversion = -99;
  unsigned multibyte_character = 0;
  if (utf8) {
    multibyte_length = mblen(utf8_text + 1, 2);
    multibyte_conversion = mbtowc(&character, utf8_text + 1, 2);
    multibyte_character = (unsigned)character;
  }
  printf("single:%d,%d,%x,%d,%d,%x\n", ascii_length, ascii_conversion,
      ascii_character, multibyte_length, multibyte_conversion, multibyte_character);
  printf("single-byte-wide:%x,%x,%d,%d,%d\n", (unsigned)btowc(EOF),
      (unsigned)btowc(0x41), wctob(WEOF), wctob(L'A'), wctob(0xe9));

  mbstate_t decode_state = {0};
  mbstate_t encode_state = {0};
  wchar_t decoded_character = 0x777;
  errno = 0;
  size_t decoded_count = mbrtowc(&decoded_character, "A", 1, &decode_state);
  int decode_error = errno;
  char encoded_bytes[16] = {0};
  errno = 0;
  size_t encoded_count = wcrtomb(encoded_bytes, L'A', &encode_state);
  printf("stateful-ascii:%lld,%x,%d,%lld,%d,", (long long)decoded_count,
      (unsigned)decoded_character, decode_error, (long long)encoded_count, errno);
  if (encoded_count != (size_t)-1) print_bytes(encoded_bytes, encoded_count);
  else putchar(';');

  if (utf8) {
    decode_state = (mbstate_t){0};
    wchar_t split_character = 0x777;
    errno = 0;
    size_t first_piece = mbrtowc(&split_character, "\xc3", 1, &decode_state);
    int first_error = errno;
    errno = 0;
    size_t second_piece = mbrtowc(&split_character, "\xa9", 1, &decode_state);
    printf("stateful-split:%lld,%d,%lld,%x,%d\n", (long long)first_piece,
        first_error, (long long)second_piece, (unsigned)split_character, errno);

    mbstate_t length_state = {0};
    errno = 0;
    size_t length_first = mbrlen("\xc3", 1, &length_state);
    int length_first_errno = errno;
    int length_pending_initial = mbsinit(&length_state);
    errno = 0;
    size_t length_second = mbrlen("\xa9", 1, &length_state);
    printf("stateful-mbrlen:%lld,%d,%d,%lld,%d,%d\n", (long long)length_first,
        length_first_errno, length_pending_initial, (long long)length_second, errno,
        mbsinit(&length_state));

    mbstate_t null_state = {0};
    size_t null_prefix = mbrtowc(&split_character, "\xc3", 1, &null_state);
    errno = 0;
    size_t null_pending = mbrtowc(NULL, NULL, 0, &null_state);
    printf("stateful-null-pending:%lld,%lld,%d\n", (long long)null_prefix,
        (long long)null_pending, errno);

    decode_state = (mbstate_t){0};
    errno = 0;
    size_t bad_count = mbrtowc(&split_character, "\xe0\x80\x80", 3, &decode_state);
    printf("stateful-invalid:%lld,%d\n", (long long)bad_count, errno);

    encode_state = (mbstate_t){0};
    errno = 0;
    size_t unicode_count = wcrtomb(encoded_bytes, 0x1f642, &encode_state);
    printf("stateful-utf8:%lld,%d,", (long long)unicode_count, errno);
    if (unicode_count != (size_t)-1) print_bytes(encoded_bytes, unicode_count);
    else putchar(';');
    errno = 0;
    size_t reset_count = wcrtomb(NULL, 0x1f642, &encode_state);
    printf("stateful-reset:%lld,%d\n", (long long)reset_count, errno);
  } else {
    printf("stateful-split:skip;stateful-mbrlen:skip;stateful-invalid:skip;stateful-utf8:skip;stateful-reset:skip\n");
  }
  mbstate_t initial_state = {0};
  printf("mbsinit:%d,%d\n", mbsinit(NULL), mbsinit(&initial_state));

  const char *restart_text = utf8 ? "A\xc3\xa9" "B" : "AB";
  const char *restart_cursor = restart_text;
  mbstate_t restart_state = {0};
  wchar_t restart_wide[8] = {0x777};
  errno = 0;
  size_t restart_query = mbsrtowcs(NULL, &restart_cursor, 0, &restart_state);
  printf("restart-decode-query:%lld,%d,%d\n", (long long)restart_query, errno,
      restart_cursor == restart_text);
  restart_cursor = restart_text;
  errno = 0;
  size_t restart_count = mbsrtowcs(restart_wide, &restart_cursor, 1, &restart_state);
  printf("restart-decode-limit:%lld,%d,%x,%d\n", (long long)restart_count, errno,
      (unsigned)restart_wide[0], restart_cursor != NULL);
  if (utf8) {
    const char bounded_text[] = {(char)0xc3, (char)0xa9, 'Z', 0};
    const char *bounded_cursor = bounded_text;
    mbstate_t bounded_state = {0};
    wchar_t bounded_wide[2] = {0x777, 0x777};
    errno = 0;
    size_t bounded_first = mbsnrtowcs(bounded_wide, &bounded_cursor, 1, 2,
        &bounded_state);
    int bounded_first_errno = errno;
    errno = 0;
    size_t bounded_second = mbsnrtowcs(bounded_wide, &bounded_cursor, 3, 2,
        &bounded_state);
    printf("bounded-split:%lld,%d,%lld,%d,%x,%x,%d\n",
        (long long)bounded_first, bounded_first_errno, (long long)bounded_second,
        errno, (unsigned)bounded_wide[0], (unsigned)bounded_wide[1],
        bounded_cursor == NULL);
  } else {
    printf("bounded-split:skip\n");
  }
  const wchar_t *restart_wide_cursor = wide_text;
  restart_state = (mbstate_t){0};
  errno = 0;
  size_t restart_encode_query = wcsrtombs(NULL, &restart_wide_cursor, 0, &restart_state);
  printf("restart-encode-query:%lld,%d,%d\n", (long long)restart_encode_query, errno,
      restart_wide_cursor == wide_text);
  char restart_bytes[16] = {0};
  restart_wide_cursor = wide_text;
  restart_state = (mbstate_t){0};
  errno = 0;
  size_t restart_encode_count = wcsrtombs(restart_bytes, &restart_wide_cursor, 1,
      &restart_state);
  printf("restart-encode-limit:%lld,%d,%02x,%d\n", (long long)restart_encode_count,
      errno, (unsigned char)restart_bytes[0], restart_wide_cursor != NULL);
  const wchar_t bounded_wide_text[] = {L'A', 0xe9, L'Z', 0};
  const wchar_t *bounded_wide_cursor = bounded_wide_text;
  mbstate_t bounded_encode_state = {0};
  errno = 0;
  size_t bounded_encode_query = wcsnrtombs(NULL, &bounded_wide_cursor, 2, 0,
      &bounded_encode_state);
  printf("bounded-encode-query:%lld,%d,%d\n", (long long)bounded_encode_query,
      errno, bounded_wide_cursor == bounded_wide_text);
  bounded_wide_cursor = bounded_wide_text;
  bounded_encode_state = (mbstate_t){0};
  char bounded_bytes[16] = {0};
  errno = 0;
  size_t bounded_encode_first = wcsnrtombs(bounded_bytes, &bounded_wide_cursor, 1,
      sizeof bounded_bytes, &bounded_encode_state);
  int bounded_encode_first_errno = errno;
  errno = 0;
  size_t bounded_encode_second = wcsnrtombs(bounded_bytes + bounded_encode_first,
      &bounded_wide_cursor, 3, sizeof bounded_bytes - bounded_encode_first,
      &bounded_encode_state);
  printf("bounded-encode-split:%lld,%d,%lld,%d,%d,", (long long)bounded_encode_first,
      bounded_encode_first_errno, (long long)bounded_encode_second, errno,
      bounded_wide_cursor == NULL);
  print_bytes(bounded_bytes, 8);

  const char *decode_inputs[] = {"AB", utf8_text, truncated_text, invalid_text};
  size_t decode_count = utf8 ? 4 : 1;
  for (size_t input = 0; input < decode_count; ++input) {
    wchar_t output[8];
    for (size_t i = 0; i < 8; ++i) output[i] = 0x777;
    errno = 0;
    size_t full = mbstowcs(output, decode_inputs[input], 8);
    int full_errno = errno;
    printf("decode:%zu,%lld,%d,", input, (long long)full, full_errno);
    print_wide(output, 8);
    for (size_t limit = 0; limit <= 4; ++limit) {
      for (size_t i = 0; i < 8; ++i) output[i] = 0x777;
      errno = 0;
      size_t converted = mbstowcs(output, decode_inputs[input], limit);
      printf("%zu,%lld,%d,", limit, (long long)converted, errno);
      print_wide(output, 5);
    }
    errno = 0;
    size_t queried = mbstowcs(NULL, decode_inputs[input], 0);
    printf("query,%lld,%d\n", (long long)queried, errno);
  }

  char bytes[16];
  for (size_t limit = 0; limit <= 7; ++limit) {
    for (size_t i = 0; i < sizeof bytes; ++i) bytes[i] = (char)0x77;
    errno = 0;
    size_t converted = wcstombs(bytes, wide_text, limit);
    printf("encode:%zu,%lld,%d,", limit, (long long)converted, errno);
    print_bytes(bytes, 8);
  }
  errno = 0;
  size_t queried = wcstombs(NULL, wide_text, 0);
  printf("encode-query:%lld,%d\n", (long long)queried, errno);

  errno = 0;
  int encoded = wctomb(bytes, 0xe9);
  printf("wctomb:%d,%d,", encoded, errno);
  if (encoded > 0) print_bytes(bytes, (size_t)encoded);
  else putchar(';');
  errno = 0;
  encoded = wctomb(bytes, 0x1f642);
  printf("wctomb-wide:%d,%d,", encoded, errno);
  if (encoded > 0) print_bytes(bytes, (size_t)encoded);
  else putchar(';');

  if (utf8) {
    wchar_t supplementary_output[2] = {0x777, 0x777};
    errno = 0;
    size_t supplementary_count = mbstowcs(supplementary_output, "\xf0\x9f\x99\x82", 2);
    printf("supplementary:%lld,%d,", (long long)supplementary_count, errno);
    print_wide(supplementary_output, 2);
  } else {
    printf("supplementary:skip;");
  }
  errno = 0;
  size_t invalid_count = wcstombs(bytes, invalid_wide, sizeof bytes);
  printf("invalid-wide:%lld,%d\n", (long long)invalid_count, errno);
  if (utf8) {
    errno = 0;
    size_t supplementary_bytes = wcstombs(bytes, supplementary_text, sizeof bytes);
    printf("supplementary-encode:%lld,%d,", (long long)supplementary_bytes, errno);
    if (supplementary_bytes != (size_t)-1) print_bytes(bytes, supplementary_bytes);
    else putchar(';');
  } else {
    printf("supplementary-encode:skip;");
  }
}

int main(void) {
  if (setlocale(LC_CTYPE, "C") == NULL) return 2;
  run_locale("C", 0);
  if (setlocale(LC_CTYPE, "C.UTF-8") != NULL) run_locale("C.UTF-8", 1);
  run_legacy_locale();
  run_stateful_locale();
  run_implicit_mbrlen();
  return 0;
}
