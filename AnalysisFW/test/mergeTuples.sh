
cwd=$(pwd)

# Environment
echo "CMSSW environment"
cd /afs/cern.ch/user/m/mhaapale/work/public/CMSSW_5_3_20
eval `scramv1 runtime -sh`
cd -

echo "EOS environment"
source /afs/cern.ch/project/eos/installation/cms/etc/setup.sh

# Mount EOS
mkdir eos
eosmount eos/
cd eos/cms/store/group/phys_smp/mhaapale/Jet/OpenDataTree_all/160705_145337/0000

# Merge files
DATA=OpenDataTree_data_*.root
NUM=$(ls -l $DATA | wc -l)
echo "Merging $NUM files..." 

hadd -f tuples0.root $DATA

cd $cwd
eosumount eos/