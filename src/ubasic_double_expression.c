#include <kernel.h>

void double_varfactor(struct ubasic_ctx* ctx, double* res)
{
	double r;

	ubasic_get_numeric_variable(tokenizer_variable_name(ctx), ctx, &r);

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
	*res = r;
}

void double_factor(struct ubasic_ctx* ctx, double* res)
{
	int tok = tokenizer_token(ctx);
	switch (tok) {
		case NUMBER:
			tokenizer_fnum(ctx, tok, res);
			accept(tok, ctx);
		break;
		case OPENBRACKET:
			accept(OPENBRACKET, ctx);
			double_expr(ctx, res);
			accept(CLOSEBRACKET, ctx);
		break;
		default:
			double_varfactor(ctx, res);
		break;
	}
}

void double_term(struct ubasic_ctx* ctx, double* res)
{
	double f1, f2;

	double_factor(ctx, &f1);

	int op = tokenizer_token(ctx);
	while (op == ASTERISK || op == SLASH || op == MOD) {
		tokenizer_next(ctx);
		double_factor(ctx, &f2);
		switch (op) {
			case ASTERISK:
				f1 = f1 * f2;
			break;
			case SLASH:
				if (f2 == 0.0) {
					tokenizer_error_print(ctx, "Division by zero");
					*res = 0.0;
				} else {
					f1 = f1 / f2;
				}
			break;
			case MOD:
				if (f2 == 0.0) {
					tokenizer_error_print(ctx, "Division by zero");
					*res = 0.0;
				} else {
					f1 = (int64_t)f1 % (int64_t)f2;
				}
			break;
		}
		op = tokenizer_token(ctx);
	}
	*res = f1;
}

void double_expr(struct ubasic_ctx* ctx, double* res)
{
	double t1, t2;

	double_term(ctx, &t1);
	int op = tokenizer_token(ctx);

	while (op == PLUS || op == MINUS || op == AND || op == OR) {
		tokenizer_next(ctx);
		double_term(ctx, &t2);
		switch (op) {
			case PLUS:
				t1 = t1 + t2;
			break;
			case MINUS:
				t1 = t1 - t2;
			break;
			case AND:
				t1 = (int64_t)t1 & (int64_t)t2;
			break;
			case OR:
				t1 = (int64_t)t1 | (int64_t)t2;
			break;
		}
		op = tokenizer_token(ctx);
	}
	*res = t1;
}

void double_relation(struct ubasic_ctx* ctx, double* res)
{
	double r1;
	double_expr(ctx, &r1);
	int op = tokenizer_token(ctx);

	while (op == LESSTHAN || op == GREATERTHAN || op == EQUALS) {
		tokenizer_next(ctx);
		double r2;
		double_expr(ctx, &r2);

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

	*res = r1;
}

