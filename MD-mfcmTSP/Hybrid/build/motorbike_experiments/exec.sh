#!/bin/bash
for i in {1..8}; do
  ./Hybrid 1 15 15 15 1 $i
  ./Hybrid 1 15 15 15 2 $i
  ./Hybrid 1 15 15 15 4 $i
done

for i in {1..8}; do
  ./Hybrid 2 15 15 15 1 $i
  ./Hybrid 2 15 15 15 2 $i
  ./Hybrid 2 15 15 15 4 $i
done

for i in {1..8}; do
  ./Hybrid 3 15 15 15 1 $i
  ./Hybrid 3 15 15 15 2 $i
  ./Hybrid 3 15 15 15 4 $i
done

for i in {1..8}; do
  ./Hybrid 4 15 15 15 1 $i
  ./Hybrid 4 15 15 15 2 $i
  ./Hybrid 4 15 15 15 4 $i
done

for i in {1..8}; do
  ./Hybrid 5 15 15 15 1 $i
  ./Hybrid 5 15 15 15 2 $i
  ./Hybrid 5 15 15 15 4 $i
done

for i in {1..8}; do
  ./Hybrid 6 15 15 15 1 $i
  ./Hybrid 6 15 15 15 2 $i
  ./Hybrid 6 15 15 15 4 $i
done

for i in {1..8}; do
  ./Hybrid 8 15 15 15 1 $i
  ./Hybrid 8 15 15 15 2 $i
  ./Hybrid 8 15 15 15 4 $i
done

for i in {1..8}; do
  ./Hybrid 9 15 15 15 1 $i
  ./Hybrid 9 15 15 15 2 $i
  ./Hybrid 9 15 15 15 4 $i
done

for i in {1..8}; do
  ./Hybrid 10 15 15 15 1 $i
  ./Hybrid 10 15 15 15 2 $i
  ./Hybrid 10 15 15 15 4 $i
done


