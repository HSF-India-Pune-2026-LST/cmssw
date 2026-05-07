# How to set up standalone LST

Hackathon version, relying on lxplus-gpu, assuming cvmfs is available and using cms-related git commands

## Setting up LST for both standalone and full framework


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

# also repeat on relogin/restart
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

## Running the code in standalone


For running the LST algorithmic code:

    lst_<backend> -i PU200 -o LSTNtuple.root

similar, exploring more arguments (check -h for all) for a quick more verbose test: `-n 10` only 10 events, `-w 0` no output root file, `-v 2` more verbose, `-s 1` single stream/queue

    lst_cuda -i PU200 -n 10 -s 1 -w 0 -v 2

Full combination, including the physics performance analysis

    lst_<backend> -i PU200 -o LSTNtuple.root # or fullInputFileName.root
    createPerfNumDenHists -i LSTNtuple.root -o LSTNumDen.root
    lst_plot_performance.py LSTNumDen.root -t "myTag" # or
    python3 efficiency/python/lst_plot_performance.py LSTNumDen.root -t "myTag"



The above can be even simplified to a single command that runs all steps

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

## Running LST at HLT within CMSSW:
(see also instructions at [this link](https://cmshltupgrade.docs.cern.ch/RunningInstructions/))

```bash
cmsDriver.py Phase2 -s L1P2GT,HLT:75e33_trackingOnly --processName=HLTX \
--conditions auto:phase2_realistic_T35 \
--geometry ExtendedRun4D121 \
--era Phase2C17I13M9 \
--eventcontent FEVTDEBUGHLT \
--customise SLHCUpgradeSimulations/Configuration/aging.customise_aging_1000 \
--filein file:/eos/cms/store/user/mmasciov/HSF-India-Pune-2026-LST/output_Phase2_L1T_RelValTTbar_PU_16_1_0_pre2_D121_1k.root \
--inputCommands='keep *, drop *_hlt*_*_HLT, drop triggerTriggerFilterObjectWithRefs_l1t*_*_HLT' \
--fileout output_HLTPhase2_baseline.root \
--mc \
--no_exec --python_filename hltTracking.py \
-n 100 --nThreads 1 --accelerators gpu-nvidia

cmsRun hltTracking.py >& mylog-hltTracking-gpu.txt&
```

## Running HLT multi-track validation:

```bash
cmsDriver.py DQM -s VALIDATION:hltMultiTrackValidation --conditions auto:phase2_realistic_T35 --geometry ExtendedRun4D121 --era Phase2C17I13M9 --datatier DQMIO --eventcontent DQM --filein file:output_HLTPhase2_baseline.root --hltProcess HLTX --fileout DQM_baseline.root -n -1 --nThreads 1 --no_exec --python_filename hltValidation.py

cmsRun hltValidation.py >& mylog-val.txt&
```

## Running harvesting:

```bash
cmsDriver.py HARVEST -s HARVESTING:@trackingOnlyValidation+@trackingOnlyDQM+postProcessorHLTtrackingSequence --filein file:DQM_baseline.root --scenario pp --filetype DQM --conditions auto:phase2_realistic_T35 --mc -n -1 --no_exec --python_filename hltHarvest.py

cmsRun hltHarvest.py >& mylog-harvest.txt&
```

## Make validation plots:
```bash
makeTrackValidationPlots.py --extended <filename #1> <filename #2>
```
