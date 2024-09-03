#!/bin/bash
for i in {1..8}; do
  ./prox 7 15 15 15 1 $i
  ./prox 7 15 15 15 2 $i
  ./prox 7 15 15 15 4 $i
done

for i in {1..8}; do
  ./prox 11 15 15 15 1 $i
  ./prox 11 15 15 15 2 $i
  ./prox 11 15 15 15 4 $i
done


