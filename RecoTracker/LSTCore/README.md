# HSF-India: LST TC ML-based selection - hackathon

## CMSSW setup:

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
# build
scram b -j 8
```

## Initialize VOMS certificate (optional):

```bash
voms-proxy-init -voms cms --valid 168:00
```

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
