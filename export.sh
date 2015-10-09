#!/bin/bash

git_root=$(git rev-parse --show-toplevel)
group_dir="/afs/cs.cmu.edu/academic/class/15410-f15-users/group-27"
project="p2"

up_to_date=$(git fetch)

if [[ $up_to_date ]]; then
    echo "Not up to date with upstream. Fetch before submitting"
    exit
fi

git_project_dir="$git_root/$project"
submit_project_dir="$group_dir/$project"

rm -r "$submit_project_dir"/*
cd $git_project_dir && make veryclean > /dev/null
cp -r "$git_project_dir"/* $submit_project_dir



