
all: antlr test

antlr:
	java -jar ../antlr-3.4-complete.jar Expr.g Eval.g

lexer:
	java -jar ../antlr-3.4-complete.jar Expr.g
	python test_lexer.py test.cpy

test:
	python cpy.py test.cpy

run:
	python a.py

clean:
	rm -rf _cpy_ *.pyc antlr3/*.pyc
