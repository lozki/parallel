/*
  +----------------------------------------------------------------------+
  | parallel                                                              |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2019                                       |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: krakjoe                                                      |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PARALLEL_EVENTS_INPUT
#define HAVE_PARALLEL_EVENTS_INPUT

#include "parallel.h"
#include "handlers.h"
#include "input.h"

zend_class_entry* php_parallel_events_input_ce;
zend_object_handlers php_parallel_events_input_handlers;

typedef struct _php_parallel_events_input_t {
    HashTable   table;
    zend_object std;
} php_parallel_events_input_t;

static zend_always_inline php_parallel_events_input_t* php_parallel_events_input_fetch(zend_object *o) {
	return (php_parallel_events_input_t*) (((char*) o) - XtOffsetOf(php_parallel_events_input_t, std));
}

static zend_always_inline php_parallel_events_input_t* php_parallel_events_input_from(zval *z) {
	return php_parallel_events_input_fetch(Z_OBJ_P(z));
}

static zend_object* php_parallel_events_input_create(zend_class_entry *type) {
    php_parallel_events_input_t *input =
        (php_parallel_events_input_t*)
            ecalloc(1, sizeof(php_parallel_events_input_t) + zend_object_properties_size(type));
            
    zend_object_std_init(&input->std, type);
    
    input->std.handlers = &php_parallel_events_input_handlers;
    
    zend_hash_init(&input->table, 32, NULL, ZVAL_PTR_DTOR, 0);
    
    return &input->std;
}

static void php_parallel_events_input_destroy(zend_object *zo) {
    php_parallel_events_input_t *input =
        php_parallel_events_input_fetch(zo);
    
    zend_hash_destroy(&input->table);    
    
    zend_object_std_dtor(zo);
}

PHP_METHOD(Input, add)
{
    php_parallel_events_input_t *input =
        php_parallel_events_input_from(getThis());
    zend_string *target;
    zval        *value;
    
    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_QUIET, 2, 2)
        Z_PARAM_STR(target)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END_EX(
        php_parallel_exception(
            "expected target and value");
        return;
    );
    
    if (!zend_hash_add(&input->table, target, value)) {
        php_parallel_exception(
            "payload for %s exists", ZSTR_VAL(target));
        return;
    }
    
    Z_TRY_ADDREF_P(value);
}

PHP_METHOD(Input, remove)
{
    php_parallel_events_input_t *input =
        php_parallel_events_input_from(getThis());
    zend_string *target;
    
    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_QUIET, 1, 1)
        Z_PARAM_STR(target)
    ZEND_PARSE_PARAMETERS_END_EX(
        php_parallel_exception(
            "expected target");
        return;
    );
    
    if (zend_hash_del(&input->table, target) != SUCCESS) {
        php_parallel_exception(
            "payload for %s does not exist", ZSTR_VAL(target));
        return;
    }
}

PHP_METHOD(Input, clear)
{
    php_parallel_events_input_t *input =
        php_parallel_events_input_from(getThis());

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_QUIET, 0, 0)
    ZEND_PARSE_PARAMETERS_END_EX(
        php_parallel_exception(
            "no parameters expected");
        return;
    );
    
    zend_hash_clean(&input->table);   
}

zend_function_entry php_parallel_events_input_methods[] = {
    PHP_ME(Input, add, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Input, remove, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Input, clear, NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

zval* php_parallel_events_input_find(zval *zv, zend_string *target) {
    php_parallel_events_input_t *input;
    
    if (Z_TYPE_P(zv) != IS_OBJECT) {
        return NULL;
    }
    
    input = php_parallel_events_input_from(zv);

    return zend_hash_find(&input->table, target);
}

zend_bool php_parallel_events_input_exists(zval *zv, zend_string *target) {
    php_parallel_events_input_t *input;
    
    if (Z_TYPE_P(zv) != IS_OBJECT) {
        return 0;
    }
    
    input = php_parallel_events_input_from(zv);
    
    return zend_hash_exists(&input->table, target); 
}

zend_bool php_parallel_events_input_remove(zval *zv, zend_string *target) {
    php_parallel_events_input_t *input;
    
    if (Z_TYPE_P(zv) != IS_OBJECT) {
        return 0;
    }
    
    input = php_parallel_events_input_from(zv);
        
    return zend_hash_del(&input->table, target) == SUCCESS;
}

void php_parallel_events_input_startup(void) {
    zend_class_entry ce;
    
    memcpy(
	    &php_parallel_events_input_handlers, 
	    php_parallel_standard_handlers(), 
	    sizeof(zend_object_handlers));

	php_parallel_events_input_handlers.offset = XtOffsetOf(php_parallel_events_input_t, std);
	php_parallel_events_input_handlers.free_obj = php_parallel_events_input_destroy;

	INIT_NS_CLASS_ENTRY(ce, "parallel\\Events", "Input", php_parallel_events_input_methods);

	php_parallel_events_input_ce = zend_register_internal_class(&ce);
	php_parallel_events_input_ce->create_object = php_parallel_events_input_create;
	php_parallel_events_input_ce->ce_flags |= ZEND_ACC_FINAL; 
}

void php_parallel_events_input_shutdown(void) {

}
#endif