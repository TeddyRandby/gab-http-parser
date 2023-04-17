#include <gab/core.h>
#include <gab/gab.h>
#include <gab/object.h>
#include <picohttpparser.h>
#include <stdio.h>

void gab_lib_parse(gab_engine *gab, gab_vm *vm, u8 argc, gab_value argv[argc]) {
  gab_value result = GAB_VAL_NIL();

  if (argc != 1 && GAB_VAL_IS_STRING(argv[0]))
    goto fin;

  gab_obj_string *req = GAB_VAL_TO_STRING(argv[0]);

  const char *method = NULL, *path = NULL;

  struct phr_header headers[100];
  int minor_version;
  size_t method_len, path_len,
      num_headers = sizeof(headers) / sizeof(headers[0]);

  int pret = phr_parse_request((char *)req->data, req->len, &method,
                               &method_len, &path, &path_len, &minor_version,
                               headers, &num_headers, 0);

  if (pret <= 0)
    goto fin;

  gab_obj_string *m =
      gab_obj_string_create(gab, s_i8_create((i8 *)method, method_len));

  gab_obj_string *p =
      gab_obj_string_create(gab, s_i8_create((i8 *)path, path_len));

  gab_value header_names[100];
  gab_value header_values[100];

  for (u32 i = 0; i < num_headers; i++) {
    s_i8 header_name = s_i8_create((i8 *)headers[i].name, headers[i].name_len);
    header_names[i] = GAB_VAL_OBJ(gab_obj_string_create(gab, header_name));

    s_i8 header_value =
        s_i8_create((i8 *)headers[i].value, headers[i].value_len);
    header_values[i] = GAB_VAL_OBJ(gab_obj_string_create(gab, header_value));
  }

  gab_value headers_shape =
      GAB_VAL_OBJ(gab_obj_shape_create(gab, vm, num_headers, 1, header_names));

  gab_value headers_record = GAB_VAL_OBJ(gab_obj_record_create(
      gab, vm, GAB_VAL_TO_SHAPE(headers_shape), 1, header_values));

  s_i8 body = s_i8_create(req->data + pret, req->len - pret);

  if (body.len > 0) {
    result = GAB_VAL_OBJ(gab_obj_string_create(gab, body));
  }

  gab_value values[] = {
      GAB_VAL_OBJ(m),
      GAB_VAL_OBJ(p),
      headers_record,
  };

  gab_push(vm, LEN_CARRAY(values), values);

fin:
  gab_push(vm, 1, &result);
}

gab_value gab_mod(gab_engine *gab, gab_vm *vm) {

  gab_value names[] = {
      GAB_STRING("to_http"),
  };

  gab_value types[] = {
      gab_type(gab, GAB_KIND_STRING),
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
