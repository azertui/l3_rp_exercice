#
# Ce Makefile contient les cibles suivantes :
#
# all		: compile les programmes
# clean		: supprime les fichiers générés automatiquement

CFLAGS = -g

PROGS = add soust div mul serv2

all: $(PROGS)

clean:
	rm -f *.o $(PROGS)
