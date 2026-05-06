# How to set up standalone LST

Hackathon version, relying on lxplus-gpu, assuming cvmfs is available and using cms-related git commands

## Setting up LST


```bash
# needed for lxplus-gpu
export SCRAM_ARCH=el9_amd64_gcc13
# (use cmsrel if you are more used to it); cd to your work location first
# repeat the following two on relogin/restart
scram p -n CMSSW_16_1_0_pre4-puneLST-sta CMSSW CMSSW_16_1_0_pre4
cd CMSSW_16_1_0_pre4-puneLST-sta/src/
cmsenv

# just once to setup
git cms-init --upstream-only -q
# If necessary, add the remote git@github.com:SegmentLinking/cmssw.git
git remote add hack-cmssw https://github.com/HSF-India-Pune-2026-LST/cmssw.git
git remote set-url --push hack-cmssw git@github.com:HSF-India-Pune-2026-LST/cmssw.git
# and checkout a development/feature branch
git cms-addpkg RecoTracker/LST RecoTracker/LSTCore
cd $CMSSW_BASE/src/RecoTracker/LSTCore/standalone/
mkdir -p tmp

# also repeat on relogin/restart
export TMPDIR=$PWD/tmp
source setup.sh
# explicit pointer for lxplus
export TRACKINGNTUPLEDIR=/eos/cms/store/user/slava77/samples/LST/CMSSW_12_2_0_pre2/

# compile standalone LST; run with -h for help; useful more debug build -mCGds
lst_make_tracklooper >& build.log &

# optionally also build CMSSW side libraries
# If modifying some dependencies, run `git cms-checkdeps -a -A`
cd $CMSSW_BASE/src
scram b -j 12 >& build.log
```

## Running the code


For running the code:

    lst_<backend> -i PU200 -o LSTNtuple.root # or fullInputFileName.root
    createPerfNumDenHists -i LSTNtuple.root -o LSTNumDen.root
    lst_plot_performance.py LSTNumDen.root -t "myTag" # or
    python3 efficiency/python/lst_plot_performance.py LSTNumDen.root -t "myTag"



The above can be even simplified

    lst_run -s PU200 -b cpu -d -t test -n 10 >& test.cpu.log & #quick test

    lst_run -f -m -s PU200 -n -1 -t myTag

The `-f` flag can be omitted when the code has already been compiled. If multiple backends were compiled, then the `-b` flag can be used to specify a backend. For example

    lst_run -b cpu -s PU200 -n -1 -t myTag

### Command explanations

Compile the code with option flags. If none of `C,G,R,A` are used, then it defaults to compiling for CUDA and CPU.

    lst_make_tracklooper -m
    -m: make clean binaries
    -C: compile CPU backend
    -G: compile CUDA backend
    -R: compile ROCm backend
    -A: compile all backends
    -h: show help screen with all options

Run the code
 
    lst_<backend> -n <nevents> -v <verbose> -w <writeout> -s <streams> -i <dataset> -o <output>

    -i: PU200; muonGun, etc [short-named samples are not setup on lxplus-gpu]
    -n: number of events; default: all
    -v: 0-no printout; 1- timing printout only; 2- multiplicity printout; default: 0
    -s: number of streams/events in flight; default: 1
    -w: 0- no writeout; 1- minimum writeout; default: 1
    -o: provide an output root file name (e.g. LSTNtuple.root); default: debug.root
    -l: add lower level object (pT3, pT5, T5, etc.) branches to the output

Plotting numerators and denominators of performance plots

    createPerfNumDenHists -i <input> -o <output> [-g <pdgids> -n <nevents>]

    -i: Path to LSTNtuple.root
    -o: provide an output root file name (e.g. num_den_hist.root)
    -n: (optional) number of events
    -g: (optional) comma separated pdgids to add more efficiency plots with different sim particle slices
    
Plotting performance plots

    lst_plot_performance.py num_den_hist.root -t "mywork"

There are several options you can provide to restrict number of plots being produced.
And by default, it creates a certain set of objects.
One can specifcy the type, range, metric, etc.
To see the full information type

    lst_plot_performance.py --help

To give an example of plotting efficiency, object type of lower level T5, for |eta| < 2.5 only.

    lst_plot_performance.py num_den_hist.root -t "mywork" -m eff -o T5_lower -s loweta

NOTE: in order to plot lower level object, ```-l``` option must have been used during ```sdl``` step!

    python3 efficiency/python/lst_plot_performance.py num_den_hist.root -t "mywork"
                                                                                                                                                           
Comparing two different runs

    lst_plot_performance.py \
        num_den_hist_1.root \     # Reference
        num_den_hist_2.root \     # New work
        -L BaseLine,MyNewWork \   # Labeling
        -t "mywork" \
        --compare

## Run the LST reconstruction in CMSSW matrix or cmsRun

NOTES ARE INCOMPLETE for Pune hackathon:

A two-iteration, tracking-only offline workflow with PU, running LST (on GPU if available, otherwise on CPU), 34634.712, has been implemented within CMSSW. More LST workflows can be found in https://github.com/cms-sw/cmssw/tree/master/Configuration/PyReleaseValidation.

To get the commands for the workflow mentioned above, one can run:

    runTheMatrix.py -w upgrade -n -e -l 34634.712

The input files in each step may need to be properly adjusted to match the ones produced by the previous step/provided externally, hence it is better to run the commands with the `--no_exec` option included.

Running the configuration file with `cmsRun`, the output file will have a name starting with `DQM`. The name is the same every time this step runs,
so it is good practice to rename the file, e.g. to `step4_34634.712.root`.
The MTV plots can be produced with the command:

    makeTrackValidationPlots.py --extended step4_34634.712.root

Comparison plots can be made by including multiple ROOT files as arguments.

## Code formatting and checking

Using the first setup option above, it is prefered to run the checks provided by CMSSW using the following commands.

```
scram b -j 12 code-checks >& c.log && scram b -j 12 code-format >& f.log
```
