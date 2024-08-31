#!/bin/bash
for i in {9..11}; do
  ./Hybrid $i 15 20 20
done
for i in {1..11}; do
  ./Hybrid $i 15 20 30
done