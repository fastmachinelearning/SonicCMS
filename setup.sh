#!/bin/bash -e

FORK=hls-fpga-machine-learning
BRANCH=abstract
CMSSWVER=CMSSW_10_6_6
ACCESS=https
PKGS=()
PKGS_ALL=(
TensorRT \
Brainwave \
)
SETUP=true

# these are used by pkg-specific setup scripts
export CORES=1
export DEBUG=""

join_by() { local IFS="$1"; shift; echo "$*"; }

usage(){
	EXIT=$1

	echo "setup.sh [options]"
	echo ""
	echo "-f [fork]           clone from specified fork (default = ${FORK})"
	echo "-b [branch]         clone specified branch (default = ${BRANCH})"
	echo "-c [version]        use specified CMSSW version (default = ${CMSSWVER})"
	echo "-p [packages]       set up specified packages (allowed = "$(join_by , "${PKGS_ALL[@]}")"; or all)"
	echo "-a [protocol]       use protocol to clone (default = ${ACCESS}, alternative = ssh)"
	echo "-j [cores]          run compilations on # cores (default = ${CORES})"
	echo "-i                  install only (to add new packages, if setup was already run)"
	echo "-d                  keep source code for debugging"
	echo "-h                  display this message and exit"

	exit $EXIT
}

# process options
while getopts "f:b:c:p:a:j:dih" opt; do
	case "$opt" in
	f) FORK=$OPTARG
	;;
	b) BRANCH=$OPTARG
	;;
	c) CMSSWVER=$OPTARG
	;;
	p) if [ "$OPTARG" = all ]; then PKGS=(${PKGS_ALL[@]}); else IFS="," read -a PKGS <<< "$OPTARG"; fi
	;;
	a) ACCESS=$OPTARG
	;;
	j) export CORES=$OPTARG
	;;
	d) export DEBUG=true
	;;
	i) SETUP=""
	;;
	h) usage 0
	;;
	esac
done

# check options
SPARSE=.git/info/sparse-checkout
git_update="git read-tree -mu HEAD"
if [ "$ACCESS" = "ssh" ]; then
	export ACCESS_GITHUB=git@github.com:
	export ACCESS_CMSSW=--ssh
elif [ "$ACCESS" = "https" ]; then
	export ACCESS_GITHUB=https://github.com/
	export ACCESS_CMSSW=--https
else
	usage 1
fi

if [ -n "$SETUP" ]; then
	export SCRAM_ARCH=slc7_amd64_gcc700
	# cmsrel
	scram project ${CMSSWVER}
	cd ${CMSSWVER}
	# cmsenv
	eval `scramv1 runtime -sh`
elif [ -z "$CMSSW_BASE" ]; then
	echo '$CMSSW_BASE required for install mode'
	exit 1
fi

# prepare for installation
unset PYTHONPATH
export WORK=$CMSSW_BASE/work
mkdir -p $WORK
cd $WORK
export LOCAL=$WORK/local
mkdir -p $LOCAL

if [ -n "$SETUP" ]; then
	# clone sonic
	cd $CMSSW_BASE/src
	git cms-init $ACCESS_CMSSW
	git clone ${ACCESS_GITHUB}${FORK}/SonicCMS -b ${BRANCH}
	git clone ${ACCESS_GITHUB}kpedro88/CondorProduction Condor/Production

	# setup sparse checkout for specified packages
	cd $CMSSW_BASE/src/SonicCMS
	git config core.sparseCheckout true
	cat << EOF_SPARSE > ${SPARSE}
/Core
/*.*
EOF_SPARSE
	$git_update
else
	cd $CMSSW_BASE/src/SonicCMS
fi

for PKG in ${PKGS[@]}; do
	echo "/$PKG" >> ${SPARSE}
done
$git_update

# loop through package-specific installations
for PKG in ${PKGS[@]}; do
	${PKG}/setup.sh
done

# build CMSSW
cd $CMSSW_BASE/src
# update env just in case
eval `scramv1 runtime -sh`
scram b -j ${CORES}
