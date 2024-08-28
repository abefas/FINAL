#!/bin/bash
for i in {31..35}; do
  ./Hybrid $i 15 15 15
done
for i in {31..35}; do
  ./Hybrid $i 15 20 20
done
for i in {31..35}; do
  ./Hybrid $i 15 20 30
done