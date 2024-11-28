PROJECT = guidelines

PDFLATEX = pdflatex
BIBTEX = bibtex

TEX_FILES := $(wildcard $(PROJECT)*.tex)
PDF_FILES := $(TEX_FILES:.tex=.pdf) 

redirect = > /dev/null

.PHONY : clean

all : $(PDF_FILES)

%.pdf : %.tex
	$(PDFLATEX) '\nonstopmode\input{$<}' $(redirect)
	@if ( grep "^\\\\bibliography{" $*.tex > /dev/null ); then \
		$(BIBTEX) -terse $(patsubst %.tex,%.aux, $<) $(redirect); \
        fi
	$(PDFLATEX) '\nonstopmode\input{$<}' $(redirect)
	$(PDFLATEX) '\nonstopmode\input{$<}' $(redirect)	

clean-all: clean
	rm -f *.ps *.eps *.pdf 

clean:
	rm -f *.log *.bak *.aux *.bbl *.blg *.idx *.toc *.lof *.lot *.out *~

