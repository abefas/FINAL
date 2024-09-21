#!/bin/bash

for i in {5..8}; do
  ./Hybrid 10 15 15 15 1 $i
  ./Hybrid 10 15 15 15 2 $i
  ./Hybrid 10 15 15 15 4 $i
done


