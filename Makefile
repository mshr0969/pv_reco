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

bin_width_pu0to200:
	root -l 'src/efficiency/bin_width/pu0to200.cpp'

save_pu200:
	root -l 'src/efficiency/save_pu200.cpp'

save_run3:
	root -l 'src/efficiency/save_run3.cpp'

save_run3_purity:
	root -l 'src/efficiency/purity/run3.cpp'

purity_run3:
	root -l 'src/efficiency/purity/run3_plot.cpp'

purity_ttbar200:
	root -l 'src/efficiency/purity/ttbar_mc200.cpp'

purity_ttbar200_plot:
	root -l 'src/efficiency/purity/ttbar_mc200_plot.cpp'

pileup:
	root -l 'src/histo/pileup.cpp'

cut:
	root -l 'src/efficiency/purity/cut.cpp'

cut_plot:
	root -l 'src/efficiency/purity/cut_plot.cpp'
