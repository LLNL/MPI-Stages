#!/bin/bash

shopt -s globstar nullglob extglob

for file in **/*.@(cc|cpp|h|hpp); do
	cat utils/header.txt $file > $file.new
	mv $file.new $file
done
