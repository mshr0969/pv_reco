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

bin_width_run3:
	root -l 'src/efficiency/bin_width/run3.cpp'

bin_width_pu200:
	root -l 'src/efficiency/bin_width/pu200.cpp'

bin_width_pu0to200:
	root -l 'src/efficiency/bin_width/pu0to200.cpp'
