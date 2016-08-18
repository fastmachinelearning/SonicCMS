
cwd=$(pwd)

# Environment
echo "CMSSW environment"
cd /afs/cern.ch/user/m/mhaapale/work/public/CMSSW_5_3_32
eval `scramv1 runtime -sh`
cd -

echo "EOS environment"
source /afs/cern.ch/project/eos/installation/cms/etc/setup.sh

# Mount EOS
echo "Mounting EOS"
mkdir -p eos
eosmount eos/

# Top-level directory of the CRAB3 outputs  
cd eos/cms/store/group/phys_smp/mhaapale/

# Output directory for merged tuples
OUTDIR="MCTuples"
mkdir -p $OUTDIR

# Input directories and files
DIRS="QCD_Pt-*to*_TuneZ2_7TeV_pythia6/"
REGEX="QCD_Pt-([[:digit:]]+to[[:digit:]]+)_TuneZ2_7TeV_pythia6/"
DATE="160728"
PREFIX="OpenDataTree_mc_"

# Loop over directories
for d in $DIRS
do
    if [[ ${d} =~ ${REGEX} ]]
    then
        echo "${BASH_REMATCH[1]}"
        # Filenames
        FILES=$d/*/${DATE}_*/*/${PREFIX}*

        # Number of files
        NUM=$(ls -l $FILES | wc -l)
        echo "Merging $NUM files..." 

        # Merging 
        hadd -f $OUTDIR/tuples_${BASH_REMATCH[1]}.root $FILES &
    fi
done

# Wait for the 'hadd' processes to finish!!
wait

cd $cwd
eosumount eos/
rmdir eos

echo "Success!"