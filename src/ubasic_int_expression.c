#include <kernel.h>

static int64_t varfactor(struct ubasic_ctx* ctx)
{
	int64_t r = ubasic_get_numeric_int_variable(tokenizer_variable_name(ctx), ctx);
	// Special case for builin functions
	if (tokenizer_token(ctx) == COMMA) {
		tokenizer_error_print(ctx, "Too many parameters for builtin function");
	} else {
		if (tokenizer_token(ctx) == CLOSEBRACKET) {
			accept(CLOSEBRACKET, ctx);
		} else {
			accept(VARIABLE, ctx);
		}
	}
	return r;
}

int64_t factor(struct ubasic_ctx* ctx)
{
	int64_t r = 0;
	int tok = tokenizer_token(ctx);
	switch (tok) {
		case NUMBER:
		case HEXNUMBER:
			r = tokenizer_num(ctx, tok);
			accept(tok, ctx);
		break;
		case OPENBRACKET:
			accept(OPENBRACKET, ctx);
			r = expr(ctx);
			accept(CLOSEBRACKET, ctx);
		break;
		default:
			r = varfactor(ctx);
		break;
	}
	return r;
}

int64_t relation(struct ubasic_ctx* ctx)
{
	int64_t r1 = expr(ctx);
	int op = tokenizer_token(ctx);

	while (op == LESSTHAN || op == GREATERTHAN || op == EQUALS) {
		tokenizer_next(ctx);
		int64_t r2 = expr(ctx);

		switch (op) {
			case LESSTHAN:
				r1 = r1 < r2;
			break;
			case GREATERTHAN:
				r1 = r1 > r2;
			break;
			case EQUALS:
				r1 = r1 == r2;
			break;
		}

		op = tokenizer_token(ctx);
	}

	return r1;
}

int64_t term(struct ubasic_ctx* ctx)
{
	int64_t f1 = factor(ctx);
	int op = tokenizer_token(ctx);
	while (op == ASTERISK || op == SLASH || op == MOD) {
		tokenizer_next(ctx);
		int64_t f2 = factor(ctx);
		switch (op) {
			case ASTERISK:
				f1 = f1 * f2;
			break;
			case SLASH:
				if (f2 == 0) {
					tokenizer_error_print(ctx, "Division by zero");
				} else {
					f1 = f1 / f2;
				}
			break;
			case MOD:
				if (f2 == 0) {
					tokenizer_error_print(ctx, "Division by zero");
				} else {
					f1 = f1 % f2;
				}
			break;
		}
		op = tokenizer_token(ctx);
	}
	return f1;
}


int64_t expr(struct ubasic_ctx* ctx)
{
	int64_t t1 = term(ctx);
	int op = tokenizer_token(ctx);

	while (op == PLUS || op == MINUS || op == AND || op == OR) {
		tokenizer_next(ctx);
		int64_t t2 = term(ctx);
		switch (op) {
			case PLUS:
				t1 = t1 + t2;
			break;
			case MINUS:
				t1 = t1 - t2;
			break;
			case AND:
				t1 = t1 & t2;
			break;
			case OR:
				t1 = t1 | t2;
			break;
		}
		op = tokenizer_token(ctx);
	}
	return t1;
}
