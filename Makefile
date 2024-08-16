include .env
export $(shell sed 's/=.*//' .env)

diff:
ifdef is_mc
	root -l 'src/histo/diff.cpp("true")'
else
	root -l 'src/histo/diff.cpp'
endif

hadd_files:
	hadd -f $(outputfile) $(inputdir)/*.root

bin_width:
	root -l 'src/efficiency/bin_width.cpp'
