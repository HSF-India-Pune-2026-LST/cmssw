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

### Initialize VOMS certificate (optional):

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

### Running HLT multi-track validation:

```bash
cmsDriver.py DQM -s VALIDATION:hltMultiTrackValidation --conditions auto:phase2_realistic_T35 --geometry ExtendedRun4D121 --era Phase2C17I13M9 --datatier DQMIO --eventcontent DQM --filein file:output_HLTPhase2_baseline.root --hltProcess HLTX --fileout DQM_baseline.root -n -1 --nThreads 1 --no_exec --python_filename hltValidation.py

cmsRun hltValidation.py >& mylog-val.txt&
```

### Running harvesting:

```bash
cmsDriver.py HARVEST -s HARVESTING:@trackingOnlyValidation+@trackingOnlyDQM+postProcessorHLTtrackingSequence --filein file:DQM_baseline.root --scenario pp --filetype DQM --conditions auto:phase2_realistic_T35 --mc -n -1 --no_exec --python_filename hltHarvest.py

cmsRun hltHarvest.py >& mylog-harvest.txt&
```

### Make validation plots:
```bash
makeTrackValidationPlots.py --extended <filename #1> <filename #2>
```

### Evaluating timing performance:

```bash
cmsDriver.py Phase2 -s L1P2GT,HLT:75e33_timing --processName=HLTX \
--conditions auto:phase2_realistic_T35 \
--geometry ExtendedRun4D121 \
--era Phase2C17I13M9 \
--eventcontent FEVTDEBUGHLT \
--customise SLHCUpgradeSimulations/Configuration/aging.customise_aging_1000 \
--filein file:/eos/cms/store/user/mmasciov/HSF-India-Pune-2026-LST/output_Phase2_L1T_RelValTTbar_PU_16_1_0_pre2_D121_1k.root \
--inputCommands='keep *, drop *_hlt*_*_HLT, drop triggerTriggerFilterObjectWithRefs_l1t*_*_HLT' \
--fileout output_HLTPhase2_baseline_time.root \
--mc \
--no_exec --python_filename hltTrackingTime.py \
-n 100 --nThreads 1 --accelerators gpu-nvidia --output={}

cmsRun hltTrackingTime.py >& mylog-hltTrackingTime-gpu.txt&
```
This will return a JSON file as output, which can be visualized with the [circles](https://github.com/cms-sw/circles) package.

## Hackathon task: filtering LST TCs with a DNN

---

## 1. What is being filtered?

In the LST pipeline, **TrackCandidates (TCs)** are intermediate objects built from detector hits.  
After TC construction and extension, their indices are **stable**, making this the correct stage to apply ML-based filtering.

Filtering TrackCandidates early:
- reduces downstream combinatorics,
- improves performance,
- enables data-driven selection strategies.

---

## 2. Key idea: filtering on the GPU

Filtering on the GPU always happens in **two distinct phases**.

### 2.1 Decision phase (masking)

A DNN is evaluated for each TrackCandidate and produces a **binary decision mask**:

tcDNNMask[i] = 1  → keep TrackCandidate i  
tcDNNMask[i] = 0  → reject TrackCandidate i

Important properties of the mask:
- one entry per TrackCandidate,
- stored in **device memory**,
- does **not** modify TrackCandidates,
- only records the DNN decision.

---

### 2.2 Compaction phase (removal)

TrackCandidates that fail the DNN are **physically removed** by:

1. allocating new containers of the correct size,
2. copying only TrackCandidates with tcDNNMask[i] == 1.

A simple `if (reject) continue;` is **not sufficient**.  
GPU arrays cannot be resized in place — compaction is required.

---

## 3. Where the code lives

### Device-side kernels
RecoTracker/LSTCore/src/alpaka/TrackCandidate.h

This file contains accelerator-agnostic kernels for:
- DNN inference,
- counting surviving TrackCandidates,
- compacting TrackCandidates.

### Host-side orchestration
RecoTracker/LSTCore/src/alpaka/LSTEvent.dev.cc

This file:
- allocates device buffers,
- launches Alpaka kernels,
- replaces old TrackCandidate containers with compacted ones.

---

## 4. Algorithm flow

Inside LSTEvent::createTrackCandidates(...), after TrackCandidates are fully built and extended, the following steps occur:

1. Allocate a device mask (tcDNNMask), one entry per TrackCandidate.
2. Run a DNN kernel to fill the mask.
3. Count how many TrackCandidates pass the DNN.
4. Allocate new TrackCandidate containers of the correct size.
5. Compact (copy) only the surviving TrackCandidates.
6. Replace the old containers with the compacted ones.

After this:
- nTrackCandidates() is reduced,
- rejected TrackCandidates no longer exist,
- downstream reconstruction sees only selected TrackCandidates.

---

## 5. DNN weights (required)

### What are the DNN weights?

The DNN relies on **pre-trained weights** (weights and biases) to compute meaningful scores.

Conceptually:

score = DNN(nHits, trackCandidateType)

The score is compared to a threshold to decide whether a TrackCandidate is kept.

---

### Where are the weights defined?

For this exercise, the DNN weights are **hard-coded in C++**.
They are defined as constant arrays inside a helper namespace (for example dnn_tc::Weights).

---
