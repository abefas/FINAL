#!/bin/bash
for i in {1..8}; do
  ./prox 1 15 15 15 1 $i
  ./prox 1 15 15 15 2 $i
  ./prox 1 15 15 15 4 $i
done

for i in {1..8}; do
  ./prox 2 15 15 15 1 $i
  ./prox 2 15 15 15 2 $i
  ./prox 2 15 15 15 4 $i
done

for i in {1..8}; do
  ./prox 3 15 15 15 1 $i
  ./prox 3 15 15 15 2 $i
  ./prox 3 15 15 15 4 $i
done

for i in {1..8}; do
  ./prox 4 15 15 15 1 $i
  ./prox 4 15 15 15 2 $i
  ./prox 4 15 15 15 4 $i
done

for i in {1..8}; do
  ./prox 5 15 15 15 1 $i
  ./prox 5 15 15 15 2 $i
  ./prox 5 15 15 15 4 $i
done

for i in {1..8}; do
  ./prox 6 15 15 15 1 $i
  ./prox 6 15 15 15 2 $i
  ./prox 6 15 15 15 4 $i
done

for i in {1..8}; do
  ./prox 8 15 15 15 1 $i
  ./prox 8 15 15 15 2 $i
  ./prox 8 15 15 15 4 $i
done

for i in {1..8}; do
  ./prox 9 15 15 15 1 $i
  ./prox 9 15 15 15 2 $i
  ./prox 9 15 15 15 4 $i
done

for i in {1..8}; do
  ./prox 10 15 15 15 1 $i
  ./prox 10 15 15 15 2 $i
  ./prox 10 15 15 15 4 $i
done


