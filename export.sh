#!/bin/bash

function usage {
    echo "./export localdir projectdir"
    echo "    localdir - copy from (path from root of git repo)"
    echo "    projectdir - copy to (path from myproj) "
}

if [ "$#" -ne 2 ]; then
    usage
    exit
fi

git_root=$(git rev-parse --show-toplevel)
group_dir="/afs/cs.cmu.edu/academic/class/15410-f15-users/group-27"

up_to_date=$(git fetch -v --dry-run)

if [[ -n $up_to_date ]]; then
    echo "Not up to date with upstream. Fetch before submitting"
    exit
fi

git_project_dir="$git_root/$1"
submit_project_dir="$group_dir/$2"

rm -rf "$submit_project_dir"/*
cd $git_project_dir && make veryclean > /dev/null
cp -r "$git_project_dir"/* $submit_project_dir
