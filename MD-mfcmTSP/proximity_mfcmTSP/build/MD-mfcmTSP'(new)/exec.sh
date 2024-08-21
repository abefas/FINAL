#!/bin/bash
for i in {1..11}; do
  ./prox $i 15 15 15
done
for i in {31..35}; do
  ./prox $i 15 15 15
done

for i in {1..11}; do
  ./prox $i 15 20 20
done
for i in {31..35}; do
  ./prox $i 15 20 20
done

for i in {1..11}; do
  ./prox $i 15 20 30
done
for i in {31..35}; do
  ./prox $i 15 20 30
done