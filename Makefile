run-timescaledb:
	@./scripts/timescaledb.sh

run-tests:
	@./scripts/build.sh tests/test.c
	@./tests/test
	@rm tests/test
	@./scripts/build.sh tests/test.cpp
	@./tests/test
	@rm tests/test
	@python3 tests/test.py
