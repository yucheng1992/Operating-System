LAB=2
# 
# NYU CS 202 - Spring 2015 - Lab
# 

# Handin
HANDIN = python submit.py
LAB_NAME = lab$(LAB)

.PHONY: tarball

handin-check:
	@if ! test -d .git; then \
		echo No .git directory, is this a git repository?; \
		false; \
	fi
	@if test "$$(git symbolic-ref HEAD)" != refs/heads/lab$(LAB); then \
		git branch; \
		read -p "You are not on the lab$(LAB) branch.  Hand-in the current branch? [y/N] " r; \
		test "$$r" = y; \
	fi
	@if ! git diff-files --quiet || ! git diff-index --quiet --cached HEAD; then \
		git status; \
		echo; \
		echo "You have uncomitted changes.  Please commit or stash them."; \
		false; \
	fi
	@if test -n "`git ls-files -o --exclude-standard`"; then \
		git status; \
		read -p "Untracked files will not be handed in. Continue? [y/N] " r; \
		test "$$r" = y; \
	fi

tarball-check: handin-check
	@for lab in $(shell seq 1 $(LAB)); do \
		$(MAKE) -C lab$${lab} realclean; \
	done

handin: tarball-check handin-check
	@git archive --format=tar HEAD > lab$(LAB)-handin.tar
	@tar -rf lab$(LAB)-handin.tar .git
	@gzip -c lab$(LAB)-handin.tar > lab$(LAB)-handin.tgz
	@rm -f lab$(LAB)-handin.tar
	@echo "  SUBMITTING ..."
	@$(HANDIN) $(LAB_NAME) lab$(LAB)-handin.tgz
