include .env
export $(shell sed 's/=.*//' .env)

diff:
	root -l src/histo/diff.cpp
