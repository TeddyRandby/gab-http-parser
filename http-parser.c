#include <gab/gab.h>
#include <gab/object.h>
#include <gab/value.h>
#include <picohttpparser.h>

gab_value gab_lib_parse(gab_engine *gab, gab_value *argv, u8 argc) {
  if (argc != 1 && GAB_VAL_IS_STRING(argv[0])) {
    return GAB_VAL_NULL();
  }

  gab_obj_string *req = GAB_VAL_TO_STRING(argv[0]);

  const char *method = NULL, *path = NULL;
  struct phr_header headers[100];
  int minor_version;
  size_t method_len, path_len,
      num_headers = sizeof(headers) / sizeof(headers[0]);

  int pret = phr_parse_request((char *)req->data, req->size, &method,
                               &method_len, &path, &path_len, &minor_version,
                               headers, &num_headers, 0);

  if (pret <= 0) {
    // failed
    return GAB_VAL_NULL();
  }

  gab_obj_string *m =
      gab_obj_string_create(gab, s_i8_create((i8 *)method, method_len));
  gab_obj_string *p =
      gab_obj_string_create(gab, s_i8_create((i8 *)path, path_len));

  s_i8 header_names[100];
  gab_value header_values[100];

  for (u32 i = 0; i < num_headers; i++) {
    header_names[i] = s_i8_create((i8 *)headers[i].name, headers[i].name_len);

    s_i8 header_value =
        s_i8_create((i8 *)headers[i].value, headers[i].value_len);

    header_values[i] = GAB_VAL_OBJ(gab_obj_string_create(gab, header_value));
  }

  gab_value h = gab_bundle(gab, num_headers, header_names, header_values);

  s_i8 keys[] = {
      s_i8_cstr("method"),
      s_i8_cstr("path"),
      s_i8_cstr("headers"),
  };

  gab_value values[] = {
      GAB_VAL_OBJ(m),
      GAB_VAL_OBJ(p),
      h,
  };

  gab_value result = gab_bundle(gab, 3, keys, values);

  gab_dref(gab, result);

  return result;
}

gab_value gab_mod(gab_engine *gab) {
  s_i8 keys[] = {
      s_i8_cstr("parse"),
  };

  gab_value values[] = {
      GAB_VAL_OBJ(gab_obj_builtin_create(gab, gab_lib_parse, "parse", 1)),
  };

  return gab_bundle(gab, 1, keys, values);
}
