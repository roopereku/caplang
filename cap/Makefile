HEADER	=	$(wildcard *.hh)
SOURCE	=	$(wildcard *.cc)

OBJECT_DEBUG	=	$(addprefix obj/debug/,$(addsuffix .o,$(SOURCE)))
OBJECT_RELEASE	=	$(addprefix obj/release/,$(addsuffix .o,$(SOURCE)))

debug:	obj/ $(OBJECT_DEBUG)

release:	obj/ $(OBJECT_RELEASE)

obj/debug/%.cc.o:	%.cc $(HEADER)
	@echo "Building $< in debug mode"
	@g++ -c -o $@ $< -std=c++17 -pedantic -Wall -Wextra -g -D DEBUG

obj/release/%.cc.o:	%.cc $(HEADER)
	@echo "Building $< in release mode"
	@g++ -c -o $@ $< -std=c++17 -pedantic -Wall -Wextra -O3

obj/:
	@mkdir -p obj/debug
	@mkdir -p obj/release

clean:
	@rm -r obj/
