#!/bin/bash
for i in {1..8}; do
  ./Hybrid 7 15 15 15 1 $i
  ./Hybrid 7 15 15 15 2 $i
  ./Hybrid 7 15 15 15 4 $i
done

for i in {1..8}; do
  ./Hybrid 11 15 15 15 1 $i
  ./Hybrid 11 15 15 15 2 $i
  ./Hybrid 11 15 15 15 4 $i
done


