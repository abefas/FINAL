#!/bin/bash
for i in {10..11}; do
  ./Hybrid $i 15 15 15
done
for i in {1..11}; do
  ./Hybrid $i 15 20 20
done