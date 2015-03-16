.PHONY: all doxygen doc clean

.SUFFIXES: .tex .pdf

PDF_DOCUMENT = doxygen.pdf
HTML_TAR_GZ = doxygen_html.tgz
TEX_DOCUMENTS = imc2ctmdp.tex

all: doxygen ${TEX_DOCUMENTS:.tex=.pdf}
doc: all

doxygen:
	@echo Doxygen...
	@doxygen doxygen.conf
	@echo Making HTML package...
	@tar -czf $(HTML_TAR_GZ) html
	@make -C latex
	@cp -f latex/refman.pdf $(PDF_DOCUMENT)

.tex.pdf:
	pdflatex -interaction nonstopmode $<
	pdflatex -interaction nonstopmode $<
	pdflatex -interaction nonstopmode $<

clean:
	-rm -rf latex html ${TEX_DOCUMENTS:.tex=.aux} ${TEX_DOCUMENTS:.tex=.log} ${TEX_DOCUMENTS:.tex=.out} ${TEX_DOCUMENTS:.tex=.toc}

