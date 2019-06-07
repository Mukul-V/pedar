#pragma once

// variable types, must be sorted (to up)
typedef enum object_type {
	TP_NULL,
	TP_CHAR,
	TP_NUMBER,
	TP_DATA,
	TP_FN,
	TP_CLASS,
	TP_IMM
} object_type_t;

typedef enum object_level {
	LEVEL_TEMPRATORY,
	LEVEL_REGISTER,
	LEVEL_REFRENCE,
	LEVEL_RESPONSE
} object_level_t;

typedef struct object {
	object_type_t type;
	object_level_t level;
	union {
		double num;
		void *ptr;
	};
} object_t;

// eval operators
enum
{
	NUL,   	// null function
	IMM,   	// immidiate value to eax
	VAR,
	CENT,   // enter to class body
	CLEV,   // leave class body
	ENT,   	// enter to body region
	LEV,   	// leave body region
	PUSH,  	// push eax to stack_frame
	POP,   	// pop stack_frame to eax
	SUPER, 	// super
	THIS,  	// this
	CONTINUE,  // continue
	BREAK, 	// break
	CHG,  	// change class path
	RST,  	// create new class path and reset it to epx
	NEW,   	// new class similar class
	CALL, 	// call a function 16
	CLST,
	JMP,   	// jump
	JZ,  	// jump if sp is not zero
	JNZ,   	// jump if sp is zero
	LD,  	// load sp to eax 21
	SD,  	// save eax to sp
	OR,
	LOR,
	XOR,
	AND,
	LAND,
	EQ,
	NE,
	LT,
	GT,
	LE,
	GE,
	LSHIFT,
	RSHIFT,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	PRTF,  	// print 41
	LOPB,
	LOPE,
	EXIT,
	SIZEOF,
	TYPEOF,

	DATA,
	DELETE,
	INSERT,
	SELECT,
	COUNT,

	SBR, // 52
	SPA,
	RAR,
	REF,
	RET,


	OPEN,
	CLOSE,
	READ,
	WRITE,
	SEEK,
	RENAME,
	CWD,
	CHDIR,
	GETS,
	GETKEY,
	WALK,

	TICK,
	TIME,

	FORMAT,

	SETUP   	// exit
};

// function types
typedef enum
{
	FN_PAREN,
	FN_BRACKET
} fn_type_t;

typedef enum
{
	CLASS_RAW,
	CLASS_BURN
} class_type_t;

/* class type */
typedef struct class {
	string_t key;

	class_type_t type;

	iarray_t *start;
	iarray_t *end;

	// parents class
	table_t *parents;
	table_t *childrens;
	table_t *functions;
	table_t *variables;

	struct class *super;
} class_t;

/* function type */
typedef struct function {
	string_t key;

	fn_type_t type;

	/* numbers of parameters */
	long64_t n;

	iarray_t *start;
	iarray_t *end;

	table_t *variables;

	struct class *super;
} function_t;


itable_t *
expression(table_t *tls, itable_t *c, array_t *code);

itable_t *
statement(table_t *tls, class_t *base, itable_t *c, class_t *clspar, array_t *code);

class_t *
parser(table_t *tls, long64_t argc, char **argv, array_t *code);
