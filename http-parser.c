#include <gab/gab.h>
#include <picohttpparser.h>
#include <stdio.h>

void gab_lib_parse(gab_engine *gab, gab_vm *vm, u8 argc, gab_value argv[argc]) {
  gab_value result = GAB_VAL_NIL();

  if (argc != 1) {
    gab_push(vm, GAB_STRING("INVALID_ARGUMENTS"));
    return;
  }

  gab_obj_string *req = GAB_VAL_TO_STRING(argv[0]);

  const char *method = NULL, *path = NULL;

  struct phr_header headers[100];
  int minor_version;
  size_t method_len, path_len,
      num_headers = sizeof(headers) / sizeof(headers[0]);

  int pret = phr_parse_request((char *)req->data, req->len, &method,
                               &method_len, &path, &path_len, &minor_version,
                               headers, &num_headers, 0);

  if (pret <= 0) {
    gab_push(vm, GAB_STRING("COULD_NOT_PARSE"));
    return;
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

  gab_value headers_record =
      gab_record(gab, vm, num_headers, header_names, header_values);

  s_i8 body = s_i8_create(req->data + pret, req->len - pret);

  if (body.len > 0) {
    result = GAB_VAL_OBJ(gab_obj_string_create(gab, body));
  }

  gab_value values[] = {
      GAB_STRING("ok"),
      GAB_VAL_OBJ(m),
      GAB_VAL_OBJ(p),
      headers_record,
  };

  gab_vpush(vm, LEN_CARRAY(values), values);
}

gab_value gab_mod(gab_engine *gab, gab_vm *vm) {

  gab_value names[] = {
      GAB_STRING("to_http"),
  };

  gab_value types[] = {
      gab_type(gab, kGAB_STRING),
  };

  gab_value values[] = {
      GAB_BUILTIN(parse),
  };

  for (i32 i = 0; i < LEN_CARRAY(values); i++) {
    gab_specialize(gab, vm, names[i], types[i], values[i]);
    gab_val_dref(vm, values[i]);
  }

  return GAB_VAL_NIL();
}
