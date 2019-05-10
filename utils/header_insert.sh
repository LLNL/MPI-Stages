#!/bin/bash

shopt -s globstar nullglob extglob

for file in include/**/*.@(cc|cpp|h|hpp); do
	cat utils/header.txt $file > $file.new
	mv $file.new $file
done

for file in src/**/*.@(cc|cpp|h|hpp); do
	cat utils/header.txt $file > $file.new
	mv $file.new $file
done
